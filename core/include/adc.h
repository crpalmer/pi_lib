#ifndef __ADC_H__
#define __ADC_H__

#include <stdint.h>

class ADC {
public:
    virtual uint16_t read(int channel) = 0;
    virtual double to_voltage(uint16_t reading) = 0;
    virtual double to_percentage(uint16_t reading) { return reading / ((double) (1<<16) - 1); }
    double read_percentage(int channel) { return to_percentage(read(channel)); }
    double read_v(int channel) { return to_voltage(read(channel)); }
};

#endif
