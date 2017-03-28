#ifndef SYSTEM_H
#define SYSTEM_H

#include <avr/wdt.h>

extern void system_init(void);
extern void system_sys_reset(void);

#define system_wdt_reset  wdt_reset

#endif
