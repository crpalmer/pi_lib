#ifndef __ADC_H__
#define __ADC_H__

#include <stdint.h>

class ADC {
public:
    virtual uint16_t read(int channel) = 0;
    virtual double to_voltage(uint16_t reading) = 0;
    double read_v(int channel) { return to_voltage(read(channel)); }
};

#endif
