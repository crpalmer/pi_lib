#include <stdio.h>
#include <stdlib.h>
#include "pi.h"
#include "pi-gpio.h"
#include "servo-gpio.h"
#include "util.h"

char buf[128];

int
main(int argc, char **argv)
{
    int c;

    pi_init();
    pi_gpio_init();

    Servo *servo = new GpioServo(22, SERVO_HS425BB_MIN, SERVO_HS425BB_MAX);
    servo->set_range(SERVO_HS425BB_MIN, SERVO_HS425BB_MAX);

    printf("Move servo to position [0, 100]:\n");
    while (pi_readline(buf, sizeof(buf))) {
	double pos;

	if (sscanf(buf, "%lf", &pos) == 1) {
	    if (servo->move_to(pos)) printf("Moved to %f\n", pos);
	} else {
	    printf("Enter a position [0, 100]\n");
	}
    }

    return 0;
}
