#ifndef __MAESTRO_H__
#define __MAESTRO_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct maestroS maestro_t;

typedef unsigned char servo_id_t;

typedef enum {
    STANDARD_SERVO,
    EXTENDED_SERVO,
    TALKING_SKULL,
    TALKING_DEER,
    TALKING_SKULL2,
    BAXTER_MOUTH,
    BAXTER_HEAD,
    BAXTER_TAIL,
    HITEC_HS65,
    HITEC_HS81,
    HITEC_HS425,
    PARALLAX_STANDARD,
    SERVO_DS3218,
} maestro_range_t;

maestro_t *
maestro_new(void);

void
maestro_destroy(maestro_t *m);

int
maestro_factory_reset(maestro_t *m);

int
maestro_n_servos(maestro_t *m);

int
maestro_set_servo_physical_range(maestro_t *m, servo_id_t id, unsigned min_us, unsigned max_us);

void
maestro_set_servo_range(maestro_t *m, servo_id_t id, maestro_range_t range);

void
maestro_set_servo_range_pct(maestro_t *m, servo_id_t id, double low, double high);

int
maestro_set_servo_speed(maestro_t *m, servo_id_t id, unsigned ms_for_range);

int
maestro_set_servo_is_inverted(maestro_t *m, servo_id_t id, int is_inverted);

int
maestro_set_servo_pos(maestro_t *m, servo_id_t id, double pos_0_100);

#ifdef __cplusplus
};

#include "servo.h"

class MaestroServo : public Servo {
public:
     MaestroServo(maestro_t *m, servo_id_t id) : m(m), id(id) { }

     bool move_to(double pos) override {
	return maestro_set_servo_pos(m, id, pos);
     }

     bool set_is_inverted(bool is_inverted) {
	return maestro_set_servo_is_inverted(m, id, is_inverted);
     }

     bool set_range(unsigned min_us, unsigned max_us) override {
	return maestro_set_servo_physical_range(m, id, min_us, max_us);
     }

     bool set_speed(unsigned ms_for_range) {
	return maestro_set_servo_speed(m, id, ms_for_range) > 0;
     }

private:
     maestro_t *m;
     servo_id_t id;
};

class Maestro {
public:
     Maestro() { m = maestro_new(); }
     ~Maestro() { maestro_destroy(m); }
     void factory_reset() { maestro_factory_reset(m); }
     int get_n_servos() { return maestro_n_servos(m); }
     MaestroServo *get_servo(servo_id_t id) { return new MaestroServo(m, id); }

private:
     maestro_t *m;
};
     
#endif

#endif
