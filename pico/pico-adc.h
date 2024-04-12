#ifndef __PICO_ADC_H__
#define __PICO_ADC_H__

#include "adc.h"
#include <hardware/adc.h>

class PicoADC : public ADC {
public:
    uint16_t read(int channel) {
	if (channel < 0 || channel > 3) return 0;
 	if (! init[channel]) {
	    adc_gpio_init(26+channel);
	    init[channel] = 0;
	}
	adc_select_input(channel);
	int total = 0;
	for (int i = 0; i < n_samples; i++) total += adc_read();
	return (total + n_samples/2) / n_samples;
    }

    double to_voltage(uint16_t reading) {
	return reading * 0.00080566; /* Resolution = 3.3V/2^12 = 3.3/4095 = 0.8mV */
    }

private:
    bool init[3] = { false, false, false };
    const int n_samples = 5;
};

#endif
