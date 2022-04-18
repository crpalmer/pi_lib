#include <stdio.h>
#include <stdlib.h>
#include "pi.h"
#include "externals/PIGPIO/pigpio.h"
#include "servo.h"
#include "util.h"

//static char buf[1024];

int
main(int argc, char **argv)
{
    int c;

    pi_init();

    gpioInitialise();
    Servo *servo = new Servo(2);

    while ((c = getchar()) >= 0) {
	double where = (c - '0') / 10.0;
	printf("go %f\n", where);
	servo->go(where);
    }

    return 0;
}
