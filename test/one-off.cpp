#include <stdio.h>
#include "pigpio.h"
#include "util.h"

int main(int argc, char **argv)
{
    gpioInitialise();
    seed_random();

    return 0;
}
