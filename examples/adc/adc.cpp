#include <stdio.h>
#include "pi.h"
#include "time-utils.h"

#define USE_ADS1115
#define PERCENT_ONLY 1

#ifdef USE_ADS1115

#include "ads1115.h"

ADC *new_adc()
{
    printf("Initializing the i2c bus.\n");

    i2c_init_bus();

    ADS1115 *adc = new ADS1115(1);
    //adc->set_max_volts(ADS1115_0_256V);
    adc->set_samples_per_sec(ADS1115_860SPS);
    return adc;
}

const int n_readings = 4;

#else

#include "pico-adc.h"
ADC *new_adc() { return new PicoADC(); }
const int n_readings = 3;

#endif

int main()
{
    pi_init_no_reboot();

    ms_sleep(1000);

    printf("Initializing the ADC.\n");
    ADC *adc = new_adc();

    printf("Reading %d channels.\n", n_readings);

    while (1) {
	for (int i = 0; i < n_readings; i++) {
	    uint16_t reading = adc->read(i);
	    double percentage = adc->to_percentage(reading);

#if PERCENT_ONLY
	    printf(" %.0f", percentage * 100);
#else
	    double v = adc->to_voltage(reading);
	    printf(" %d:%d:%.2f:%.0f", i, reading, v, percentage * 100);
#endif

	}
	printf("\n");
	ms_sleep(500);
    }
}
