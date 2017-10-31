#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_DRIVER_NULL

#include "debug.h"
#include "driver.h"
#include "drv_null.h"

static u08 null_init(u08 did)
{
  DS("dNi"); DNL;
  return DRIVER_OK;
}

static u16 null_read(u08 did, u08 *buf, u16 size)
{
  DS("dNr:"); DW(size); DNL;
  return size;
}

static u16 null_write(u08 did, u08 *buf, u16 size)
{
  DS("dNw:"); DW(size); DNL;
  return size;
}

DRIVER_BEGIN(null)
  .init_func = null_init,
  .read_func = null_read,
  .write_func = null_write,
  .mtu_max = 0,
  .mtu_min = 0
DRIVER_END
