#ifndef __STEPPER_H__
#define __STEPPER_H__

#define STEPPER_N_COILS 4

typedef struct stepperS stepper_t;

stepper_t *stepper_new(int gpios[STEPPER_N_COILS], int step_delay_ms);

void stepper_wait_complete(stepper_t *s);

void stepper_forward(stepper_t *s, unsigned n);

void stepper_backward(stepper_t *s, unsigned n);

void stepper_destroy(stepper_t *s);

#endif

