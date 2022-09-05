#include <hardware/adc.h>
#include "light-sensor.h"

LightSensor::LightSensor(int id) : id(id) {
    adc_gpio_init(26+id);
}

double LightSensor::get() {
    adc_select_input(id);
    return ((double) adc_read()) / (1 << 12);
}

