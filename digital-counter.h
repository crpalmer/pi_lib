#ifndef __DIGITAL_COUNTER_H__
#define __DIGITAL_COUNTER_H__

/* wiring:

   You need the wb to have a high side driver for the selected bank.
   You need to have it set to 5V power if you are using a 5V relay.
   You then connect inc/dec to the indicated pins.
   You then connect the relay with
      - gnd and signal attached to gnd
      - +V attached to the indicated pin
 */

typedef struct digital_counterS digital_counter_t;

digital_counter_t *
digital_counter_new(int bank, int inc, int dec, int reset);

void digital_counter_set(digital_counter_t *, int new_value);

void digital_counter_add(digital_counter_t *, int n);

void digital_counter_reset(digital_counter_t *);

void digital_counter_free(digital_counter_t *);

#endif
