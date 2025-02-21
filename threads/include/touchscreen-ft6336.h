#ifndef __TOUCHSCREEN_FT6336_H__
#define __TOUCHSCREEN_FT6336_H__

#include "thread-interrupt-notifier.h"

typedef struct {
    int id;
    int x, y;
    struct timespec touch_at;
} touch_event_t;

class TouchscreenEventHandler : public PiThread {
public:
    TouchscreenEventHandler(int i2c, int touch_delta = 3, const char *name = "event-handler");

    void set_touch_state(bool is_touched);
    void main() override;

    virtual void on_touch(touch_event_t event) { }
    virtual void on_released(touch_event_t event) { }

private:
    static const int MAX_TOUCHES = 2;

    int i2c;
    int touch_delta;

    PiMutex *lock;
    PiCond *cond;
    bool is_touched = false;

    uint8_t n_touches = 0;
    touch_event_t touches[MAX_TOUCHES * 2];
    int touch_id = 0;

    int report_touches();
    int read_pos(int i2c, int reg, uint16_t *val);
    bool is_same_touch_as(int x, int y, touch_event_t *event);
    bool is_same_point(int a, int b);
};

class TouchscreenInterruptNotifier : public ThreadInterruptNotifier {
public:
    TouchscreenInterruptNotifier(Input *interrupt, TouchscreenEventHandler *event_handler) : interrupt(interrupt), event_handler(event_handler) {
        interrupt->set_notifier(this);
    }

    void on_change_safe() {
	event_handler->set_touch_state(interrupt->get());
    }

private:
    Input *interrupt;
    TouchscreenEventHandler *event_handler;
};

#endif
