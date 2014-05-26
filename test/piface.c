#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "piface.h"

int
main(int argc, char **argv)
{
    piface_t *p;
    int i;

    p = piface_new();
    while(true) {
	printf("Push button 0..."); fflush(stdout);

	while (true) {
	   piface_wait_for_input(p);
	   if (piface_get(p, 0)) {
	        printf("  Thank you.\n");
		break;
	   } else if (piface_get_all(p)) {
	        printf("  You naughty little beast.\n");
		break;
	   }
	}

	for (i = 0; i < 8; i++) {
	    piface_set(p, i, true);
	    usleep(100*1000);
	    piface_set(p, i, false);
	}
    }

    return 0;
}
