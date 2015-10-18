#include <stdio.h>
#include "pi-usb.h"
#include "maestro.h"

#define POLOLU_VENDOR_ID        0x1ffb
#define MAESTRO_PRODUCT_ID      0x0089

#define REQUEST_GET_PARAMETER 0x81
#define REQUEST_SET_PARAMETER 0x82
#define REQUEST_GET_VARIABLES 0x83
#define REQUEST_SET_SERVO_VARIABLE 0x84
#define REQUEST_SET_TARGET 0x85
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

#define PARAMETER_SERIAL_MODE 3
#define PARAMETER_SERVO_HOME(id) (30+(id)*9)
#define PARAMETER_SERVO_MIN(id) (32+(id)*9)
#define PARAMETER_SERVO_MAX(id) (33+(id)*9)
#define PARAMETER_SERVO_NEUTRAL(id) (34+(id)*9)
#define PARAMETER_SERVO_RANGE(id) (36+(id)*9)
#define PARAMETER_SERVO_SPEED(id) (37+(id)*9)
#define PARAMETER_SERVO_ACCELERATION(id) (38+(id)*9)

typedef struct {
    servo_id_t		servo_id;
    unsigned short	home;
    unsigned short	min_pos;
    unsigned short	max_pos;
    unsigned short	neutral;
    unsigned char	range;
    double		speed;
    unsigned char	acceleration;
    int			is_inverted;
} servo_config_t;

struct maestroS {
    struct usb_device	  *dev;
    struct usb_dev_handle *handle;
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
set_raw_parameter_ushort(maestro_t *m, int parameter, unsigned short value)
{
    char val_bytes[2];

    val_bytes[0] = value & 0xff;
    val_bytes[1] = (value >> 8) & 0xff;

    if (usb_control_msg(m->handle, 0xc0, REQUEST_SET_PARAMETER, 0, parameter, val_bytes, 2, -1) < 0) {
	fprintf(stderr, "usb_control_msg: %s\n", usb_strerror());
	return 0;
    }

    return 1;
}

static int
get_servo_config(maestro_t *m, servo_id_t id, servo_config_t *c)
{
    unsigned char pos_tmp;
    unsigned char speed_tmp;

    if (id >= m->n_servos) return 0;

    c->servo_id = id;
    if (! get_raw_parameter_ushort(m, PARAMETER_SERVO_HOME(id), &c->home)) return 0;
    if (! get_raw_parameter_byte(m, PARAMETER_SERVO_MIN(id), &pos_tmp)) return 0;
    c->min_pos = pos_tmp << 6;
    if (! get_raw_parameter_byte(m, PARAMETER_SERVO_MAX(id), &pos_tmp)) return 0;
    c->max_pos = pos_tmp << 6;
    if (! get_raw_parameter_ushort(m, PARAMETER_SERVO_NEUTRAL(id), &c->neutral)) return 0;
    if (! get_raw_parameter_byte(m, PARAMETER_SERVO_RANGE(id), &c->range)) return 0;
    if (! get_raw_parameter_byte(m, PARAMETER_SERVO_SPEED(id), &speed_tmp)) return 0;
    c->speed = (speed_tmp >> 3) * (2 << (speed_tmp & 0x7));

    fprintf(stderr, "servo %d: %d..%d @ %f\n", id, c->min_pos, c->max_pos, c->speed);

    return 1;
}

maestro_t *
maestro_new(void)
{
    struct usb_device *dev = pi_usb_device(POLOLU_VENDOR_ID, MAESTRO_PRODUCT_ID);
    maestro_t *m;
    unsigned i;

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
    for (i = 0; i < m->n_servos; i++) {
	get_servo_config(m, i, &m->c[i]);
    }

    return m;
}

void
maestro_destroy(maestro_t *m)
{
    free(m->c);
    usb_close(m->handle);
    free(m);
}

int
maestro_n_servos(maestro_t *m)
{
    return m->n_servos;
}

int
maestro_set_servo_speed(maestro_t *m, servo_id_t id, unsigned ms_for_full_range)
{
    double speed;

    if (id >= m->n_servos) return 0;

    if (ms_for_full_range == 0) {
	speed = 0;
    } else {
	unsigned short total_us;
	double total_units;

	total_us = m->c[id].max_pos - m->c[id].min_pos + 1;
	total_units = total_us / 0.25;
	speed = total_units / (ms_for_full_range / 10.0);
    }

    return set_raw_parameter_ushort(m, PARAMETER_SERVO_SPEED(id), speed);
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
    unsigned short real_pos;

    if (id >= m->n_servos) return 0;
    if (pos > 100) return 0;

    if (m->c[id].is_inverted) pos = 100 - pos;

    real_pos = ((m->c[id].max_pos - m->c[id].min_pos) * pos + 50) / 100 + m->c[id].min_pos;
    return usb_control_msg(m->handle, 0x40, REQUEST_SET_TARGET, real_pos, id, NULL, 0, -1) >= 0;
}

void
maestro_set_servo_range(maestro_t *m, servo_id_t id, maestro_range_t range)
{
    switch(range) {
    case STANDARD_SERVO:
	m->c[id].min_pos = 1050*4;
	m->c[id].max_pos = 1950*4;
	break;
    case EXTENDED_SERVO:
	m->c[id].min_pos = 600*4;
	m->c[id].max_pos = 2400*4;
	break;
    case TALKING_SKULL:
	m->c[id].min_pos = 1000*4;
	m->c[id].max_pos = 1680*4;
	break;
    case TALKING_DEER:
	// TODO, this is a placeholder
	m->c[id].min_pos = 1400*4;
	m->c[id].max_pos = 1800*4;
	break;
    }
}
