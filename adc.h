#ifndef __ADC_H__
#define __ADC_H__

class ADC {
public:
    virtual uint16_t read(int channel) = 0;
    virtual double to_voltage(uint16_t reading) = 0;
};

#endif
