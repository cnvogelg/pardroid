#include <stdint.h>
#include "kinetis.h"

#include "autoconf.h"
#include "hw_system.h"
#include "hw_timer.h"
#include "eeprom.h"

static uint32_t last_ws;

void hw_system_init(void)
{
  eeprom_initialize();
}

void hw_system_reset(void)
{
  // enable
  WDOG_STCTRLH = WDOG_STCTRLH_ALLOWUPDATE | WDOG_STCTRLH_WDOGEN;
  // 500 ms
  WDOG_TOVALL = 500;
  WDOG_TOVALH = 0;
  WDOG_PRESC = 0; // 1KHz dog timer

  // fastest way to trigger a watchdog reset:
  // write invalid value to UNLOCK register
  WDOG_UNLOCK = 0;

  // wait for my death
  while(1) {}
}
