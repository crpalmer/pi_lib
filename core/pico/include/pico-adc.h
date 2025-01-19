#ifndef __PICO_ADC_H__
#define __PICO_ADC_H__

#include "adc.h"

class PicoADC : public ADC {
public:
    uint16_t read(int channel);
    double to_voltage(uint16_t reading);
    double to_percentage(uint16_t reading) override { return reading / 4095.0; }

private:
    bool init[3] = { false, false, false };
    const int n_samples = 5;
};

#endif
