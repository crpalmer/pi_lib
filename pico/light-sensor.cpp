#include <hardware/adc.h>
#include "light-sensor.h"

LightSensor::LightSensor(int id) {
    LightSensor(id, light_sensor_generic);
}

LightSensor::LightSensor(int id, light_sensor_type_t type) : id(id), type(type) {
    adc_gpio_init(26+id);
}

double LightSensor::get_raw() {
    adc_select_input(id);
    return ((double) adc_read()) / (1 << 12);
}

double LightSensor::get_lux() {
    double raw = get_raw();
    switch (type) {
    case light_sensor_generic: return raw*1000;
    case light_sensor_temt6000:
	double volts = 3.3 * raw;
	double amps = volts / 10000;	// 10K ohms in the package?
	double microamps = amps * 1000000;
	double lux = microamps * 2;
	return lux;
    }
    return 0;
}
