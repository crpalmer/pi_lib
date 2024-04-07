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
    return adc;
}

const int n_readings = 4;

#else

#include "pico-adc.h"
ADC *new_adc() { return new PicoADC(); }
const int n_readings = 1;

#endif

#define THRESHOLD 1.0

int main()
{
    pi_init();
    struct timespec start;

    ms_sleep(2000);

    i2c_init_bus(1);
    i2c_config_gpios(2, 3);

    ADC *adc = new_adc();

    nano_gettime(&start);

    while (1) {
	int n = 0;
	for (int i = 0; i < n_readings; i++) {
	    double v;

	    if ((v = adc->read_v(i)) >= THRESHOLD) {
		if (n == 0) printf("%5.3f", nano_elapsed_ms_now(&start) / 1000.0);
		printf(" %d:%.2f", i, v);
		n++;
	    }
	}
	if (n) {
	    printf("\n");
	    ms_sleep(100);
	}
    }
}
