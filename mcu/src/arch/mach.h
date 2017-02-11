#ifndef MACH_H
#define MACH_H

#include <avr/wdt.h>

extern void mach_init_hw(void);
extern void mach_sys_reset(void);

#define mach_wdt_reset  wdt_reset

#endif
