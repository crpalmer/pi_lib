#include <stdio.h>
#include "pi.h"
#include "pico-adc.h"
#include "util.h"

int main()
{
    pi_init();

    ADC *adc = new PicoADC();

    while (1) {
	printf("%.02f %.02f %.02f\n", adc->to_voltage(adc->read(0)), adc->to_voltage(adc->read(1)), adc->to_voltage(adc->read(2)));
	ms_sleep(100);
    }
}
