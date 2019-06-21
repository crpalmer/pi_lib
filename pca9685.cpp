#include <stdio.h>
#include "pca9685.h"
#include "pigpio.h"
#include "util.h"

#define MODE1_RESET_BIT		0x80
#define MODE1_EXTCLK_BIT	0x40
#define MODE1_AUTO_INC_BIT	0x20
#define MODE1_SLEEP_BIT		0x10
#define MODE1_SUB1_BIT		0x08
#define MODE1_SUB2_BIT		0x04
#define MODE1_SUB3_BIT		0x02
#define MODE1_ALLCALL_BIT	0x01

#define PCA9685_SUBADR1 0x2
#define PCA9685_SUBADR2 0x3
#define PCA9685_SUBADR3 0x4

#define PCA9685_LED0_ADDR	0x06

#define PCA9685_MODE1 0x0
#define PCA9685_PRESCALE 0xFE

#define LED0_ON_L 0x6

PCA9685::PCA9685(unsigned address, unsigned hz)
{
    bus = i2cOpen(1, address, 0);
    if (bus < 0) {
        throw "failed to open i2c device";
    }

    reset();
    set_pwm_freq(hz);
}

PCA9685::~PCA9685()
{
    i2cClose(bus);
}

void PCA9685::reset()
{
    i2cWriteByteData(bus, PCA9685_MODE1, 0x80);
    ms_sleep(10);
}

void PCA9685::set_pwm_freq(unsigned hz)
{
    double freq = 0.9 * hz;
    double prescale_real = 25000000 / 4096 / freq - 1;
    unsigned prescale = (unsigned) (prescale_real + 0.5);
    unsigned char mode;

    this->hz = hz;

    mode = i2cReadByteData(bus, PCA9685_MODE1);
    i2cWriteByteData(bus, PCA9685_MODE1, (mode & 0x7f) | MODE1_SLEEP_BIT);
    i2cWriteByteData(bus, PCA9685_PRESCALE, prescale);
    i2cWriteByteData(bus, PCA9685_MODE1, mode);
    ms_sleep(5);
    i2cWriteByteData(bus, PCA9685_MODE1, mode | MODE1_AUTO_INC_BIT);
}

output_t *PCA9685::get_output(unsigned id)
{
    return new PCA9685_output_t(this, id);
}

void PCA9685::set(unsigned id, double freq)
{
    unsigned addr = PCA9685_LED0_ADDR + id*4;
    unsigned on_at = 0;
    unsigned on_count = 0;

    if (freq == 0) on_count = 4096;
    else if (freq >= 1) on_at = 4096;
    else on_count = (unsigned) (freq * 4096);
	
    i2cWriteByteData(bus, addr + 0, on_at & 0xff);
    i2cWriteByteData(bus, addr + 1, on_at >> 8);
    i2cWriteByteData(bus, addr + 2, on_count & 0xff);
    i2cWriteByteData(bus, addr + 3, on_count >> 8);
}
