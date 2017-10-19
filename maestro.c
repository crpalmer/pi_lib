#include <stdio.h>
#include <fcntl.h>
#include "pi-usb.h"
#include "maestro.h"
#include "util.h"

#define DEBUG 0

#define MAX_SERIAL_DEVICES	10
#define SERIAL_DEVICE_FMT	"/dev/ttyACM%d"

#define POLOLU_VENDOR_ID        0x1ffb
#define MAESTRO_PRODUCT_ID      0x0089

// USB serial messages for normal control
#define COMMAND_SET_TARGET		0x84
#define COMMAND_SET_SPEED		0x87
#define COMMAND_SET_ACCELERATION	0x89
#define COMMAND_GET_POSITION		0x90
#define COMMAND_GET_MOVING_STATE	0x93
#define COMMAND_GET_ERRORS		0xA1
#define COMMAND_GO_HOME			0xA2

// USB control messages for hardward setup
#define REQUEST_GET_PARAMETER 0x81
#define REQUEST_SET_PARAMETER 0x82
#define REQUEST_GET_VARIABLES 0x83
#define REQUEST_SET_SERVO_VARIABLE 0x84
#define REQUEST_CLEAR_ERRORS 0x86
#define REQUEST_GET_SERVO_SETTINGS 0x87

// GET STACK and GET CALL STACK are only used on the Mini Maestro
#define REQUEST_GET_STACK 0x88
#define REQUEST_GET_CALL_STACK 0x89
#define REQUEST_SET_PWM 0x8a

#define REQUEST_REINITIALIZE 0x90
#define REQUEST_ERASE_SCRIPT 0xa0
#define REQUEST_WRITE_SCRIPT 0xa1
#define REQUEST_SET_SCRIPT_DONE 0xa2, // value.low.b is 0 for go, 1 for stop, 2 for single-step
#define REQUEST_RESTART_SCRIPT_AT_SUBROUTINE 0xA3
#define REQUEST_RESTART_SCRIPT_AT_SUBROUTINE_WITH_PARAMETER 0xA4
#define REQUEST_RESTART_SCRIPT 0xa5
#define REQUEST_START_BOOTLOADER 0xff

/* ============================ parameters ======================== */

#define PARAMETER_INITIALIZED 0
#define PARAMETER_SERIAL_MODE 3
#define PARAMETER_SERVO_HOME(id) (30+(id)*9)
#define PARAMETER_SERVO_MIN(id) (32+(id)*9)
#define PARAMETER_SERVO_MAX(id) (33+(id)*9)
#define PARAMETER_SERVO_NEUTRAL(id) (34+(id)*9)
#define PARAMETER_SERVO_RANGE(id) (36+(id)*9)
#define PARAMETER_SERVO_SPEED(id) (37+(id)*9)
#define PARAMETER_SERVO_ACCELERATION(id) (38+(id)*9)

#define SERIAL_MODE_USB 0

#define SERVO_POS_MULTIPLIER 4

typedef struct {
    servo_id_t		servo_id;
    unsigned short	home;
    unsigned short	min_pos_us;
    unsigned short	max_pos_us;
    unsigned short	neutral;
    unsigned char	range;
    unsigned char	acceleration;
    int			is_inverted;
    unsigned short	current_real_pos;
} servo_config_t;

struct maestroS {
    struct usb_device	  *dev;
    struct usb_dev_handle *handle;
    int			   fd;
    int			   n_servos;
    servo_config_t	  *c;
};

static int
get_raw_parameter_byte(maestro_t *m, int parameter, unsigned char *ret)
{
    char result;

    if (usb_control_msg(m->handle, 0xc0, REQUEST_GET_PARAMETER, 0, parameter, &result, 1, -1) < 0) {
	fprintf(stderr, "usb_control_msg: %s\n", usb_strerror());
	return 0;
    }

    *ret = result;

    return 1;
}

static int
get_raw_parameter_ushort(maestro_t *m, int parameter, unsigned short *ret)
{
    char result[2];

    if (usb_control_msg(m->handle, 0xc0, REQUEST_GET_PARAMETER, 0, parameter, result, 2, -1) < 0) {
	fprintf(stderr, "usb_control_msg: %s\n", usb_strerror());
	return 0;
    }

    *ret = ((unsigned char) result[0]) | (((unsigned char) result[1]) << 8);

    return 1;
}

static int
set_raw_parameter_byte(maestro_t *m, int parameter, unsigned char value)
{
    unsigned short index = (1<<8) | parameter;

    if (usb_control_msg(m->handle, 0x40, REQUEST_SET_PARAMETER, value, index, NULL, 0, -1) < 0) {
	fprintf(stderr, "usb_control_msg: %s\n", usb_strerror());
	return 0;
    }

    return 1;
}

#if 0
static int
set_raw_parameter_ushort(maestro_t *m, int parameter, unsigned short value)
{
    unsigned short index = (2<<8) | parameter;

    if (usb_control_msg(m->handle, 0x40, REQUEST_SET_PARAMETER, value, index, NULL, 0, -1) < 0) {
	fprintf(stderr, "usb_control_msg: %s\n", usb_strerror());
	return 0;
    }

    return 1;
}
#endif

int
send_cmd_ushort(maestro_t *m, servo_id_t id, unsigned char cmd, unsigned short data)
{
    unsigned char bytes[4];

    bytes[0] = cmd;
    bytes[1] = id;
    bytes[2] = data%128;
    bytes[3] = data/128;

    if (write(m->fd, bytes, 4) != 4) {
	perror("write");
	return 0;
    }

    return 1;
}


static int
get_servo_config(maestro_t *m, servo_id_t id, servo_config_t *c)
{
    unsigned char pos_tmp;

    if (id >= m->n_servos) return 0;

    c->servo_id = id;
    if (! get_raw_parameter_ushort(m, PARAMETER_SERVO_HOME(id), &c->home)) return 0;
    if (! get_raw_parameter_byte(m, PARAMETER_SERVO_MIN(id), &pos_tmp)) return 0;
    c->min_pos_us = pos_tmp * 64 / SERVO_POS_MULTIPLIER;
    if (! get_raw_parameter_byte(m, PARAMETER_SERVO_MAX(id), &pos_tmp)) return 0;
    c->max_pos_us = pos_tmp * 64 / SERVO_POS_MULTIPLIER;
    if (! get_raw_parameter_ushort(m, PARAMETER_SERVO_NEUTRAL(id), &c->neutral)) return 0;
    if (! get_raw_parameter_byte(m, PARAMETER_SERVO_RANGE(id), &c->range)) return 0;

    if (DEBUG) printf("config %d: home %d pos %d..%d neutral %d range %d\n", id, c->home, c->min_pos_us, c->max_pos_us, c->neutral, c->range);
    return 1;
}

static void
get_all_servos_config(maestro_t *m)
{
    unsigned i;

    for (i = 0; i < m->n_servos; i++) {
	m->c[i].current_real_pos = 0;
	get_servo_config(m, i, &m->c[i]);
    }
}

static int
restart_controller(maestro_t *m)
{
    if (usb_control_msg(m->handle, 0x40, REQUEST_REINITIALIZE, 0, 0, NULL, 0, -1) < 0) {
	fprintf(stderr, "usb_control_msg: %s\n", usb_strerror());
	return 0;
    }

    ms_sleep(1500);

    get_all_servos_config(m);

    return 1;
}

maestro_t *
maestro_new(void)
{
    struct usb_device *dev = pi_usb_device(POLOLU_VENDOR_ID, MAESTRO_PRODUCT_ID);
    maestro_t *m;
    unsigned char serial_mode;
    int i;

    if (! dev) return NULL;

    m = malloc(sizeof(*m));
    m->dev = dev;
    switch(dev->descriptor.idProduct) {
    case 0x89: m->n_servos = 6; break;
    case 0x8a: m->n_servos = 12; break;
    case 0x8b: m->n_servos = 18; break;
    case 0x8c: m->n_servos = 24; break;
    default: free(m); return NULL;
    }

    m->handle = usb_open(dev);

    m->c = calloc(sizeof(*m->c), m->n_servos);
    get_all_servos_config(m);

    if (get_raw_parameter_byte(m, PARAMETER_SERIAL_MODE, &serial_mode) && serial_mode != SERIAL_MODE_USB) {
	fprintf(stderr, "WARNING: serial mode %d is not usb mode, resetting it\n", serial_mode);
	set_raw_parameter_byte(m, PARAMETER_SERIAL_MODE, SERIAL_MODE_USB);
	restart_controller(m);
    }

    for (i = 0; i < MAX_SERIAL_DEVICES; i++) {
	char serial_device[128];

	sprintf(serial_device, SERIAL_DEVICE_FMT, i);
	if ((m->fd = open(serial_device, O_RDWR | O_NOCTTY)) < 0) {
	    perror(serial_device);
	} else {
	    break;
	}
    }

    if (i >+ MAX_SERIAL_DEVICES) {
	maestro_destroy(m);
	return NULL;
    }

    for (i = 0; i < m->n_servos; i++) {
	maestro_set_servo_range(m, i, STANDARD_SERVO);
	maestro_set_servo_speed(m, i, 0);
    }

    return m;
}

void
maestro_destroy(maestro_t *m)
{
    free(m->c);
    usb_close(m->handle);
    if (m->fd >= 0) close(m->fd);
    free(m);
}

int
maestro_n_servos(maestro_t *m)
{
    return m->n_servos;
}

int
maestro_set_servo_speed(maestro_t *m, servo_id_t id, unsigned ms_for_range)
{
    unsigned speed;

    if (id >= m->n_servos) return 0;

    if (ms_for_range == 0) {
	speed = 0;
    } else {
	unsigned short total_us;
	double total_units;

	total_us = m->c[id].max_pos_us - m->c[id].min_pos_us + 1;
	total_units = total_us / 0.25;
	speed = total_units / (ms_for_range / 10.0);
    }

    if (DEBUG) printf("speed %d => %d\n", ms_for_range, speed);

    return send_cmd_ushort(m, id, COMMAND_SET_SPEED, speed);

    return 0;
}

int
maestro_set_servo_is_inverted(maestro_t *m, servo_id_t id, int is_inverted)
{
    if (id >= m->n_servos) return 0;

    m->c[id].is_inverted = is_inverted;
    return 1;
}

int
maestro_set_servo_pos(maestro_t *m, servo_id_t id, double pos)
{
    unsigned short real_pos, real_pos_us;
    double real_pos_real;

    if (id >= m->n_servos) return 0;
    if (pos > 100) return 0;

    if (m->c[id].is_inverted) pos = 100 - pos;

    real_pos_real = (m->c[id].max_pos_us - m->c[id].min_pos_us) * pos / 100.0;
    real_pos_us = (unsigned short) (real_pos_real + 0.5) + m->c[id].min_pos_us;
    real_pos = real_pos_us * SERVO_POS_MULTIPLIER;

    if (DEBUG) printf("real_pos = %d (%d us)\n", real_pos, real_pos_us);

    if (m->c[id].current_real_pos == real_pos) {
	return 1;
    } else {
	m->c[id].current_real_pos = real_pos;
        return send_cmd_ushort(m, id, COMMAND_SET_TARGET, real_pos);
    }
}

int
maestro_set_servo_physical_range(maestro_t *m, servo_id_t id, unsigned min_us, unsigned max_us)
{
    if (! set_raw_parameter_byte(m, PARAMETER_SERVO_MIN(id), min_us/64*SERVO_POS_MULTIPLIER)) return 0;
    if (! set_raw_parameter_byte(m, PARAMETER_SERVO_MAX(id), max_us/64*SERVO_POS_MULTIPLIER)) return 0;
    get_servo_config(m, id, &m->c[id]);
    return 1;
}

void
maestro_set_servo_range(maestro_t *m, servo_id_t id, maestro_range_t range)
{
    switch(range) {
    case STANDARD_SERVO:
	maestro_set_servo_physical_range(m, id, 1050, 1950);
	break;
    case EXTENDED_SERVO:
	maestro_set_servo_physical_range(m, id, 600, 2400);
	break;
    case TALKING_SKULL:
	maestro_set_servo_range_pct(m, id, 30, 75);
	maestro_set_servo_is_inverted(m, id, 1);
	break;
    case TALKING_DEER:
	maestro_set_servo_range_pct(m, id, 30, 60);
	break;
    case TALKING_SKULL2:
	maestro_set_servo_range_pct(m, id, 15, 33);
	maestro_set_servo_is_inverted(m, id, 1);
	break;
    case BAXTER_MOUTH:
	maestro_set_servo_range(m, id, HITEC_HS425);
	maestro_set_servo_range_pct(m, id, 0, 25);
	maestro_set_servo_speed(m, id, 250);
	break;
    case BAXTER_HEAD:
	maestro_set_servo_range(m, id, PARALLAX_STANDARD);
	maestro_set_servo_range_pct(m, id, 0, 75);
	break;
    case BAXTER_TAIL:
	maestro_set_servo_range(m, id, HITEC_HS81);
	//maestro_set_servo_range_pct(m, id, 25, 75);
	break;
    case HITEC_HS65:
	maestro_set_servo_physical_range(m, id, 610, 2360);
	break;
    case HITEC_HS81:
	maestro_set_servo_physical_range(m, id, 640, 2250);
	break;
    case HITEC_HS425:
	maestro_set_servo_physical_range(m, id, 553, 2520);
	break;
    case PARALLAX_STANDARD:
	maestro_set_servo_physical_range(m, id, 750, 2250);
	break;
    }
}

void
maestro_set_servo_range_pct(maestro_t *m, servo_id_t id, double low, double high)
{
    double scale = (m->c[id].max_pos_us - m->c[id].min_pos_us + 1) / 100.0;

    if (m->c[id].is_inverted) {
	double o_low = low, o_high = high;
	low = 100 - o_high;
	high = 100 - o_low;
    }

    m->c[id].max_pos_us = m->c[id].min_pos_us + scale*high;
    m->c[id].min_pos_us = m->c[id].min_pos_us + scale*low;
}

int
maestro_factory_reset(maestro_t *m)
{
    if (! set_raw_parameter_byte(m, PARAMETER_INITIALIZED, 0xff)) return 0;
    restart_controller(m);
    ms_sleep(1500);
    return 1;
}
