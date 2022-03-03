#include <stdint.h>
#include "kinetis.h"

#include "autoconf.h"
#include "hw_system.h"
#include "hw_timer.h"
#include "eeprom.h"

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

void hw_system_init(void)
{
  // watchdog already setup

  eeprom_initialize();
}

void hw_system_sys_reset(void)
{
  // fastest way to trigger a watchdog reset:
  // write invalid value to UNLOCK register
  WDOG_UNLOCK = 0;

  // wait for my death
  while(1) {}
}

void hw_system_wdt_reset(void)
{
  // only trigger ws reset at least every ms
  uint32_t m = hw_timer_millis();
  if((m - last_ws) > 2) {
    last_ws = m;

    __disable_irq();
    WDOG_REFRESH = 0xA602;
    WDOG_REFRESH = 0xB480;
    __enable_irq();
  }
}
