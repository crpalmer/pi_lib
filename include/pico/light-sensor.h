#ifndef __LIGHT_SENSOR_H__
#define __LIGHT_SENSOR_H__

typedef enum {
    light_sensor_generic,
    light_sensor_temt6000
} light_sensor_type_t;

class LightSensor {
public:
    LightSensor(int id);
    LightSensor(int id, light_sensor_type_t type);

    double get_raw();
    double get_lux();

private:
    int id;
    light_sensor_type_t type;
};

#endif
