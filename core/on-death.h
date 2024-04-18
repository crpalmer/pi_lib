#ifndef __ON_DEATH_H__
#define __ON_DEATH_H__

template <class X>
class OnDeath {
public:
    virtual void on_death(X object);
};

#endif
