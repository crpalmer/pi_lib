#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "pi.h"
#include "distance.h"
#include "gp-input.h"
#include "gp-output.h"

int
main()
{
    pi_init();

    GPOutput *trigger = new GPOutput(0);
    GPInput *echo = new GPInput(1);

    printf("Looping\n");
    while (1) {
	printf("%d\n", ping_distance_cm(trigger, echo));
	ms_sleep(500);
    }
}
