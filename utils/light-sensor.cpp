#include <stdio.h>
#include "light-sensor.h"
#include "pi.h"
#include "util.h"

int
main(int argc, char **argv)
{
    pi_init();

    LightSensor *l = new LightSensor(2);

    while (1) {
	printf("%3.0f\n", l->get_lux());
	ms_sleep(500);
    }
}
