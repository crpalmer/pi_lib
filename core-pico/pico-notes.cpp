#include <stdio.h>
#include "pi-gpio.h"
#include "time-utils.h"

#include "notes.h"

void Notes::play(note_t *notes, int n_notes, int time_scaling)
{
    for (int i = 0; i < n_notes; i++) {
	int ms = notes[i].ms * time_scaling;

	pi_gpio_pwm_enable(gpio, notes[i].frequency);
	pi_gpio_pwm_set_duty(gpio, 0.5);
	ms_sleep(ms/2);
	pi_gpio_pwm_disable(gpio);
	ms_sleep(ms - ms/2);
    }
}
