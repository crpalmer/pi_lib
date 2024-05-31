#ifndef __SERVO_H__
#define __SERVO_H__

#define SERVO_STANDARD_MIN 1050
#define SERVO_STANDARD_MAX 1950

#define SERVO_HS425BB_MIN   553
#define SERVO_HS425BB_MAX  2520

#define SERVO_STRETCH_MIN   900
#define SERVO_STRETCH_MAX  2100

#define SERVO_EXTENDED_MIN  500
#define SERVO_EXTENDED_MAX 2500

#define SERVO_ST

class Servo {
public:
    virtual ~Servo() {}
    virtual bool move_to(double pos_from_0_to_100) { return false; }
    virtual bool set_is_inverted(bool is_inverted) { return false; }
    virtual bool set_range(unsigned mn, unsigned mx) { return false; }
    virtual bool set_speed(unsigned ms_for_range) { return false; }
};

#endif
