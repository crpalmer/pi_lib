#ifndef PICO_BATTERY_H
#define PICO_BATTERY_H

#ifdef __cplusplus
extern "C" {
#endif

bool pico_is_on_battery();
double pico_get_vsys();

#ifdef __cplusplus
};
#endif

#endif
