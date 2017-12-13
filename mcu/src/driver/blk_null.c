#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_DRIVER_NULL

#include "debug.h"
#include "driver.h"
#include "blk_null.h"

static u08 init(u08 did)
{
  DS("dNi"); DNL;
  return DRIVER_OK;
}

static u08 blk_read(u08 did, u08 slot, u32 blk_no, u08 *buf)
{
  DS("null:r:"); DL(blk_no); DNL;
  return DRIVER_OK;
}

static u08 blk_write(u08 did, u08 slot, u32 blk_no, const u08 *buf)
{
  DS("null:w:"); DL(blk_no); DNL;
  return DRIVER_OK;
}

static u08 blk_get_capacity(u08 did, u08 slot, u32 *num_blocks)
{
  *num_blocks = 1024;
  DS("null:c:"); DL(*num_blocks); DNL;
  return DRIVER_OK;
}

DRIVER_BEGIN(blk_null)
  .init_func = init,
  .blk_read_func = blk_read,
  .blk_write_func = blk_write,
  .blk_get_capacity_func = blk_get_capacity,
  // consts
  .type = DRIVER_TYPE_BLK,
  .num_slots = DRIVER_MAX_SLOTS
DRIVER_END
