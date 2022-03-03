#include "hardware/watchdog.h"

#include "autoconf.h"
#include "hw_system.h"

void hw_system_init(void)
{
  // 500ms 
  watchdog_enable(500, true);
}

void hw_system_sys_reset(void)
{
  watchdog_enable(1, true);
  // wait for my death
  while(1) {}
}

void hw_system_wdt_reset(void)
{
  watchdog_update();
}
