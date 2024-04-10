#include <stdio.h>
#include <hardware/clocks.h>
#include <hardware/pwm.h>
#include <hardware/gpio.h>
#include <pico/time.h>
#include "mem.h"
#include "notes.h"

Notes::Notes(unsigned gpio, int max_queue_size) {
    this->gpio = gpio;
    this->max_queue_size = max_queue_size;
 
    push_at = pop_at = 0;
    alarm_id = -1;

    notes = (note_t *) fatal_malloc(sizeof(*notes) * max_queue_size);
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(gpio);
}

static int64_t alarm_callback(alarm_id_t alarm_id, void *arg)
{
    Notes *notes = (Notes *) arg;
    return notes->play_next();
}

void Notes::play(note_t *new_notes, int n_notes, int time_scaling)
{
    for (int i = 0; i < n_notes; i++) {
	if (next_index(push_at) != pop_at) {
	    notes[push_at] = new_notes[i];
	    notes[push_at].ms *= time_scaling;
	    push_at = next_index(push_at);
	}
    }

    if (alarm_id < 0) {
	play_next();
    }
}

int64_t Notes::play_next()
{
    if (delay_next_us) {
	int64_t ret = delay_next_us;
	delay_next_us = 0;
        pwm_set_chan_level(slice_num, PWM_CHAN_A, 0);
	return ret;
    }

    if (push_at == pop_at) {
	alarm_id = -1;
	return 0;
    }

    note_t note = notes[pop_at];
    int64_t note_us = note.ms * 1000;
    pop_at = next_index(pop_at);

    delay_next_us = note_us / 10;
    note_us -= delay_next_us;

    if (alarm_id < 0) {
        alarm_id = add_alarm_in_us(note_us, alarm_callback, this, true);
    }

    pwm_config cfg = pwm_get_default_config();

    if (note.frequency) {
        unsigned count = (125*1000*1000) * 16 / note.frequency;
        unsigned div = count / 60000;  // to be lower than 65535*15/16 (rounding error)
        unsigned top = count / div;
    
        if(div < 16) div = 16;
        cfg.div = div;
        cfg.top = top;
    
        pwm_init(slice_num, &cfg, true);
        pwm_set_chan_level(slice_num, PWM_CHAN_A, top / 2);
    } else {
        pwm_set_chan_level(slice_num, PWM_CHAN_A, 0);
    }

    return note_us;
}
