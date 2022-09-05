#ifndef __LIGHT_SENSOR_H__
#define __LIGHT_SENSOR_H__

#include <hardware/adc.h>

class LightSensor {
public:
    LightSensor(int id);
    double get();

private:
    int id;
};

#endif
