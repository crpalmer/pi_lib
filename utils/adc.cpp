#include <stdio.h>
#include "pi.h"
#include "time-utils.h"
#include "util.h"

#define USE_ADS1115

#ifdef USE_ADS1115

#include "ads1115.h"

ADC *new_adc()
{
    ADS1115 *adc = new ADS1115(1);
    //adc->set_max_volts(ADS1115_0_256V);
    adc->set_samples_per_sec(ADS1115_860SPS);
    return adc;
}

const int n_readings = 4;

#else

#include "pico-adc.h"
ADC *new_adc() { return new PicoADC(); }
const int n_readings = 1;

#endif

#define THRESHOLD 1.00

int main()
{
    pi_init();

    ms_sleep(2000);

    printf("Initializing the i2c bus.\n");

    i2c_init_bus(1);
    i2c_config_gpios(2, 3);

    printf("Initializing the ADC.\n");
    ADC *adc = new_adc();

    printf("Reading %d channels.\n", n_readings);

    while (1) {
        struct timespec start;
        nano_gettime(&start);

	int n = 0;

	for (int i = 0; i < n_readings; i++) {
	    double v;

	    v = adc->read_v(i);
	    if (v  > THRESHOLD) {
		printf(" %d:%.2f", i, v);
		n++;
	    }
	}
	if (n > 0) printf(" [%.3f]\n", nano_elapsed_ms_now(&start) / 1000.0);
	ms_sleep(10);
    }
}
