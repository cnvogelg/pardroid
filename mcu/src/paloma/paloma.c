#include "types.h"
#include "arch.h"
#include "autoconf.h"

#include "handler.h"
#include "driver.h"
#include "channel.h"

void paloma_init(void)
{
  channel_init();

  u08 drv_num = DRIVER_GET_TABLE_SIZE();
  driver_reset(drv_num);
  driver_init(drv_num);

  u08 hnd_num = HANDLER_GET_TABLE_SIZE();
  handler_init(hnd_num);
}

void paloma_handle(void)
{
  u08 drv_num = DRIVER_GET_TABLE_SIZE();
  driver_work(drv_num);

  u08 hnd_num = HANDLER_GET_TABLE_SIZE();
  handler_work(hnd_num);
}
