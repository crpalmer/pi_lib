#ifndef __CONSOLES_H__
#define __CONSOLES_H__

#ifdef __cplusplus
extern "C" {
#endif

int consoles_write_str(const char *str);
int consoles_printf(const char *fmt, ...);

#ifdef __cplusplus
};

#include "console.h"
#include "consoles-lock.h"

void consoles_add(Console *c);
void consoles_on_death(Console *c);
void consoles_set_consoles_lock(ConsolesLock *lock);

#endif

#endif
