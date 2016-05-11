#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "wb.h"

static void output_test(int bank)
{
    wb_t *wb;

    wb = wb_new();
    while (true) {
	int i;

	for (i = 0; i < 8; i++) {
	    printf("enable output %d\n", i+1);
	    wb_set(wb, WB_OUTPUT(bank, i), 1);
	    sleep(1);
	    wb_set(wb, WB_OUTPUT(bank, i), 0);
	}
    }
}

static void input_test(void)
{
    wb_t *wb;
    int last = -1;

    wb = wb_new();
    while (true) {
	int cur = wb_get_all(wb);
	if (cur != last) {
	    int pin;

	    last = cur;
	    for (pin = 7; pin >= 0; pin--) {
		printf("%d", (cur & (1<<pin)) != 0);
	    }
	    printf("\n");
	}
    }
}

int
main(int argc, char **argv)
{
    int bank;

    if (argc == 3 && strcmp(argv[1], "--out") == 0 && sscanf(argv[2], "%d", &bank) == 1) {
	if (bank < 1 || bank > 2) {
	     fprintf(stderr, "Invalid output bank, must be either 0 or 1\n");
	     exit(1);
	}
	output_test(bank-1);
    } else if (argc != 1) {
	fprintf(stderr, "usage: [--out bank]\n");
	exit(1);
    } else {
	input_test();
    }
    exit(0);
}
