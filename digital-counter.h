#ifndef __DIGITAL_COUNTER_H__
#define __DIGITAL_COUNTER_H__

#ifdef __cplusplus

#include "io.h"

class digital_counter_t {
public:
    digital_counter_t(output_t *inc, output_t *dec, output_t *reset);
    ~digital_counter_t();

    void add(int delta);
    void set(unsigned value);
    void set_pause(int pause = -1, int reset_pause = -1, int post_reset_pause = -1);

private:
    static void *thread_main(void *);

    output_t *inc, *dec, *reset;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    pthread_t thread;
    int target, actual;
    bool stop;
    unsigned pause, reset_pause, post_reset_pause;
};

#else
extern "C" {

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
digital_counter_new(unsigned bank, unsigned inc, unsigned dec, unsigned reset);

void
digital_counter_set_pause(digital_counter_t *, int pause, int reset_pause, int post_reset_pause);

void digital_counter_set(digital_counter_t *, int new_value);

void digital_counter_add(digital_counter_t *, int n);

void digital_counter_reset(digital_counter_t *);

void digital_counter_free(digital_counter_t *);

};
#endif

#endif
