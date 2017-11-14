#include <stdint.h>
#include "kinetis.h"

#include "autoconf.h"
#include "system.h"

void system_init(void)
{
    // setup watchdog
    // unlock
    WDOG_UNLOCK = WDOG_UNLOCK_SEQ1;
    WDOG_UNLOCK = WDOG_UNLOCK_SEQ2;
    __asm__ volatile ("nop");
    __asm__ volatile ("nop");
    // enable
    WDOG_STCTRLH = WDOG_STCTRLH_ALLOWUPDATE | WDOG_STCTRLH_WDOGEN;
    // 500 ms
    WDOG_TOVALL = 500;
    WDOG_TOVALH = 0;
    WDOG_PRESC = 0; // 1KHz dog timer
}

void system_sys_reset(void)
{
    // unlock
    WDOG_UNLOCK = WDOG_UNLOCK_SEQ1;
    WDOG_UNLOCK = WDOG_UNLOCK_SEQ2;
    __asm__ volatile ("nop");
    __asm__ volatile ("nop");
    // enable
    WDOG_STCTRLH = WDOG_STCTRLH_ALLOWUPDATE | WDOG_STCTRLH_WDOGEN;
    // 1 ms
    WDOG_TOVALL = 1;
    WDOG_TOVALH = 0;
    WDOG_PRESC = 0; // 1KHz dog timer
    // wait for my deatch
    while(1) {}
}

void system_wdt_reset(void)
{
    WDOG_REFRESH = 0xA602;
    WDOG_REFRESH = 0xB480;
}
