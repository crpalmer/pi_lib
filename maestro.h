#ifndef __MAESTRO_H__
#define __MAESTRO_H__

typedef struct maestroS maestro_t;

typedef unsigned char servo_id_t;

maestro_t *
maestro_new(void);

void
maestro_destroy(maestro_t *m);

int
maestro_n_servos(maestro_t *m);

int
maestro_set_servo_is_inverted(maestro_t *m, servo_id_t id, int is_inverted);

int
maestro_set_servo_pos(maestro_t *m, servo_id_t id, unsigned char pos_0_100);

#endif