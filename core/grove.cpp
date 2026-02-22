#include "pi.h"
#include "i2c.h"
#include "grove.h"

const unsigned char speed_addr = 0x82;
const unsigned char pwm_addr = 0x84;
const unsigned char direction_addr = 0xaa;

GroveDC::GroveDC(unsigned addr)
{
    i2c = i2c_open(1, addr);
    if (i2c < 0) {
	fprintf(stderr, "Could not communicate with i2c device\n");
	exit(1);
    }
    speeds[0] = 0;
    speeds[1] = 0;
    dirs[0] = 0x01;
    dirs[1] = 0x01;
    i2c_write_word(i2c, pwm_addr, 0x0002);
    set_speed();
    set_direction();
}

void GroveDC::direction(unsigned id, int dir)
{
    assert_id(id);
    assert(dir != 0);
    dirs[id] = dir < 0 ? 0x10 : 0x01;
    set_direction();
}

void GroveDC::speed(unsigned id, unsigned speed)
{
    assert_id(id);
    assert(speed <= 255);

    speeds[id] = speed;
    set_speed();
}

void GroveDC::set_speed()
{
    i2c_write(i2c, speed_addr, (char *) speeds, 2);
}

void GroveDC::set_direction()
{
    i2c_write(i2c, direction_addr, (char *) dirs, 2);
}

GroveStepper::GroveStepper(unsigned addr, unsigned phases) : phases(phases)
{
    i2c = i2c_open(1, addr);
    if (i2c < 0) {
	fprintf(stderr, "Could not communicate with i2c device\n");
	exit(1);
    }
    cur_step = 0;
    dir = 1;
    set_step();
    assert(phases == 2 || phases == 4);
    i2c_write_word(i2c, pwm_addr, 0x0002);
    i2c_write_word(i2c, speed_addr, 0xffff);
    ms_sleep(4);
}

void GroveStepper::direction(int dir)
{
    assert(dir == -1 || dir == 1);
    this->dir = dir;
}

void GroveStepper::set_direction(unsigned short dir)
{
printf("%d %d%d%d%d\n", cur_step, (dir & 0b1000) != 0, (dir & 0b0100) != 0, (dir & 0b0010) != 0, (dir & 0b0001) != 0);
    i2c_write_word(i2c, direction_addr, dir);
}

void GroveStepper::set_step()
{
#if 0
    if (phases == !2) {
	if (dir > 0) {
	    switch (cur_step) {
	    case 0: set_direction(0b0001); ms_sleep(10); set_direction(0b0101); break;
	    case 1: set_direction(0b0100); ms_sleep(10); set_direction(0b0110); break;
	    case 2: set_direction(0b0010); ms_sleep(10); set_direction(0b1010); break;
	    case 3: set_direction(0b1000); ms_sleep(10); set_direction(0b1001); break;
	    }
	} else {
	    switch (cur_step) {
	    case 0: set_direction(0b0001); ms_sleep(10); set_direction(0b1001); break;
	    case 1: set_direction(0b0100); ms_sleep(10); set_direction(0b0101); break;
	    case 2: set_direction(0b0010); ms_sleep(10); set_direction(0b0110); break;
	    case 3: set_direction(0b1000); ms_sleep(10); set_direction(0b1010); break;
	    }
	}
    } else {
	if (dir > 0) {
	    switch (cur_step) {
	    case 0: set_direction(0b0001); ms_sleep(10); set_direction(0b0011); break;
	    case 1: set_direction(0b0010); ms_sleep(10); set_direction(0b0110); break;
	    case 2: set_direction(0b0100); ms_sleep(10); set_direction(0b1100); break;
	    case 3: set_direction(0b1000); ms_sleep(10); set_direction(0b1001); break;
	    }
	} else {
	    switch (cur_step) {
	    case 0: set_direction(0b0001); ms_sleep(10); set_direction(0b1100); break;
	    case 1: set_direction(0b0010); ms_sleep(10); set_direction(0b0011); break;
	    case 2: set_direction(0b0100); ms_sleep(10); set_direction(0b0110); break;
	    case 3: set_direction(0b1000); ms_sleep(10); set_direction(0b1100); break;
	    }
	}
   }
#else
   unsigned short steps[4] = { 0b1010, 0b0101, 0b0110, 0b1001 };
   set_direction(steps[cur_step]);
#endif
}

void GroveStepper::step()
{
    cur_step += dir;
    if (cur_step < 0) cur_step = 3;
    else if (cur_step > 3) cur_step = 0;
    set_step();
}
