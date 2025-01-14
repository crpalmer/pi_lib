#ifndef __CONSOLES_H__
#define __CONSOLES_H__

#ifdef __cplusplus
extern "C" {
#endif

int consoles_write_str(const char *str);
int consoles_printf(const char *fmt, ...);
void consoles_fatal_printf(const char *fmt, ...);

#ifdef __cplusplus
};

class Console;
void consoles_add(Console *c);
void consoles_remove(Console *c);

#include "console.h"
#include "consoles-lock.h"

void consoles_set_consoles_lock(ConsolesLock *lock);

#endif

#endif
