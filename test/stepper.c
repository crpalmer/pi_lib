#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "stepper.h"

static int gpios[4] = { 26, 19, 13, 6 };

int
main(int argc, char **argv)
{
    stepper_t *s = stepper_new(gpios, 2);
    int dir = 0;

    while (1) {
	time_t start = time(NULL);
	while (time(NULL) - start < 60) {
	    if (dir) stepper_forward(s, 1);
	    else stepper_backward(s, 1);
	}
	dir = !dir;
   }
}
