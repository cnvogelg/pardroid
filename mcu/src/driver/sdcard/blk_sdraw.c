#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_DRIVER_SDCARD

#include "debug.h"
#include "driver.h"
#include "blk_sdraw.h"
#include "sdcard.h"
#include "uartutil.h"

static u08 init(u08 did)
{
  uart_send_pstring(PSTR("sdcard: "));
  u08 result = sdcard_init();
  if(result == SDCARD_RESULT_OK) {
    uart_send_pstring(PSTR("OK!"));
    uart_send_crlf();
    return DRIVER_OK;
  } else {
    uart_send_hex_byte(result);
    uart_send_crlf();
    return DRIVER_ERROR_INIT_FAILED;
  }
}

static u08 blk_read(u08 did, u08 slot, u32 blk_no, u08 *buf)
{
  u08 status = sdcard_read(blk_no, buf);
  if(status != SDCARD_RESULT_OK) {
    return DRIVER_ERROR_READ;
  } else {
    return DRIVER_OK;
  }
}

static u08 blk_write(u08 did, u08 slot, u32 blk_no, const u08 *buf)
{
  u08 status = sdcard_write(blk_no, buf);
  if(status != SDCARD_RESULT_OK) {
    return DRIVER_ERROR_WRITE;
  } else {
    return DRIVER_OK;
  }
}

static u08 blk_get_capacity(u08 did, u08 slot, u32 *num_blocks)
{
  u08 status = sdcard_get_capacity(num_blocks);
  if(status != SDCARD_RESULT_OK) {
    return DRIVER_ERROR_READ;
  } else {
    return DRIVER_OK;
  }
}

DRIVER_BEGIN(blk_sdraw)
  .init_func = init,
  .blk_read_func = blk_read,
  .blk_write_func = blk_write,
  .blk_get_capacity_func = blk_get_capacity,
  // consts
  .type = DRIVER_TYPE_BLK,
  .num_slots = 1,
  .blk_size = 512
DRIVER_END
