#ifndef __CONSOLES_LOCK_H__
#define __CONSOLES_LOCK_H__

class ConsolesLock {
public:
    virtual void lock() { }
    virtual void unlock() { }
};

#endif
