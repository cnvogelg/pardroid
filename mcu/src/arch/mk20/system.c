#include <stdint.h>
#include "kinetis.h"

#include "autoconf.h"
#include "system.h"
#include "timer.h"

static uint32_t last_ws;

void startup_early_hook(void)
{
  // enable
  WDOG_STCTRLH = WDOG_STCTRLH_ALLOWUPDATE | WDOG_STCTRLH_WDOGEN;
  // 500 ms
  WDOG_TOVALL = 500;
  WDOG_TOVALH = 0;
  WDOG_PRESC = 0; // 1KHz dog timer
}

void system_init(void)
{
  // watchdog already setup
}

void system_sys_reset(void)
{
  __disable_irq();
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
  __enable_irq();
  // wait for my death
  while(1) {}
}

void system_wdt_reset(void)
{
  // only trigger ws reset at least every ms
  uint32_t m = timer_millis();
  if((m - last_ws) > 2) {
    last_ws = m;

    __disable_irq();
    WDOG_REFRESH = 0xA602;
    WDOG_REFRESH = 0xB480;
    __enable_irq();
  }
}
