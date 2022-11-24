#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "debug.h"

#include "hw_uart.h"
#include "hw_system.h"
#include "hw_timer.h"

#include "uartutil.h"
#include "rominfo.h"
#include "fwid.h"
#include "fw_info.h"

#include "disk.h"
#include "disk_svc.h"
#include "disk_map.h"
#include "disk_driver.h"

FW_INFO(FWID_TEST_DISK, VERSION_TAG)

//#ifdef FLAVOR_DEBUG
//#define VERBOSE
//#endif

// ----- test disk functions -----

static u08 my_open(u08 unit, u08 flags)
{
  return DISK_OK;
}

static void my_close(u08 unit)
{
}

static u08 my_get_size(u08 unit, disk_size_t *size)
{
  return DISK_OK;
}

static u08 my_get_geo(u08 unit, disk_geo_t *geo)
{
  return DISK_OK;
}

static u08 my_read(u08 unit, u32 lba, u16 num_blocks, u08 *data)
{
  return DISK_OK;
}

static u08 my_write(u08 chn, u32 lba, u16 num_blocks, u08 *data)
{
  return DISK_OK;
}

// ----- declare test disk driver -----

DISK_DRIVER_BEGIN(test_driver)
  .config.device = DISK_DEVICE_TEST,
  .config.num_units = 4,
  .config.name = PSTR("test_driver"),

  .open = my_open,
  .close = my_close,

  .get_size = my_get_size,
  .get_geo = my_get_geo,

  .read = my_read,
  .write = my_write
DISK_DRIVER_END

DISK_DRIVER_TABLE_BEGIN
  &test_driver
DISK_DRIVER_TABLE_END

// ----- tables with disk service -----

REQ_HANDLER_TABLE_BEGIN
  &disk_svc_ctl,
REQ_HANDLER_TABLE_END

HANDLER_TABLE_BEGIN
  ADD_REQ_HANDLER(disk_svc_ctl),
  &disk_svc_data
HANDLER_TABLE_END

int main(void)
{
  hw_system_init();
  hw_uart_init();

  uart_send_pstring(PSTR("parbox: test-disk!"));
  uart_send_crlf();

  rom_info();

  pamela_init();

  disk_map_init();

  disk_map_slot_t slot = { DISK_DEVICE_TEST, 0, DISK_FLAGS_NONE };
  disk_map_set_slot(0, &slot);

  while(1) {
    pamela_work();
#ifdef VERBOSE
    uart_send('.');
#endif
 }

  return 0;
}
