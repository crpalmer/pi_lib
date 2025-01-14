#ifndef __DEEP_SLEEP_H__
#define __DEEP_SLEEP_H__

#ifdef __cplusplus
extern "C" {
#endif

void pico_enter_deep_sleep_until(int gpio);

#ifdef __cplusplus
};

#include "pi-threads.h"
#include "time-utils.h"

class DeepSleeper : public PiThread {
public:
    DeepSleeper(int wakeup_gpio, int sleep_after_ms, const char *thread_name = "deep-sleep");
    void prod();
    void main() override;
    virtual void pre_sleep() {}
    virtual void post_sleep() {}

private:
    int wakeup_gpio;

protected:
    const int sleep_after_ms;
    struct timespec last_press;

private:
    int check_ms;
};

#endif

#endif
