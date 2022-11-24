#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_DISK

#include "debug.h"

#include "disk.h"
#include "disk_driver.h"

/* import driver table */
DISK_DRIVER_TABLE_DECLARE

/* find driver by device */
static disk_driver_ptr_t find_driver(u08 device)
{
  for(u08 id=0;id<DISK_DRIVER_TABLE_GET_SIZE();id++) {
    disk_driver_ptr_t driver = DISK_DRIVER_TABLE_GET_ENTRY(id);
    if(DISK_DRIVER_GET_DEVICE(driver) == device) {
      return driver;
    }
  }
  return NULL;
}

u08 disk_open(disk_handle_t *hnd, u08 device, u08 unit, u08 flags)
{
  /* first try to find device driver */
  disk_driver_ptr_t driver = find_driver(device);
  if(driver == NULL) {
    return DISK_ERROR_UNKNOWN_DEVICE;
  }

  /* check if unit is valid */
  if(unit > DISK_MAX_UNITS) {
    return DISK_ERROR_INVALID_UNIT;
  }
  if(unit > DISK_DRIVER_GET_NUM_UNITS(driver)) {
    return DISK_ERROR_INVALID_UNIT;
  }

  /* check if unit is busy */
  disk_driver_data_t *data = DISK_DRIVER_GET_DATA(driver);
  u08 unit_mask = 1 << unit;
  if((data->unit_mask & unit_mask) != 0) {
    return DISK_ERROR_UNIT_BUSY;
  }

  /* try to open unit in driver */
  u08 result = DISK_DRIVER_FUNC_OPEN(driver)(unit, flags);
  if(result != DISK_OK) {
    return result;
  }

  /* mark unit as open */
  data->unit_mask |= unit_mask;

  /* fill handle */
  hnd->device = device;
  hnd->unit = unit;
  hnd->flags = flags;
  hnd->driver = driver;

  return DISK_OK;
}

u08 disk_close(disk_handle_t *hnd)
{
  if(hnd->driver == NULL) {
    return DISK_ERROR_NOT_OPEN;
  }

  /* check if unit is still open */
  disk_driver_data_t *data = DISK_DRIVER_GET_DATA(hnd->driver);
  u08 unit_mask = 1 << hnd->unit;
  if((data->unit_mask & unit_mask) == 0) {
    return DISK_ERROR_INVALID_UNIT;
  }

  /* clear unit */
  data->unit_mask &= ~unit_mask;

  /* call close in driver */
  DISK_DRIVER_FUNC_CLOSE(hnd->driver)(hnd->unit);

  /* clear handle */
  hnd->device = DISK_DEVICE_NONE;
  hnd->unit = DISK_MAX_UNITS;
  hnd->flags = 0;
  hnd->driver = NULL;

  return DISK_OK;
}

u08 disk_get_size(disk_handle_t *hnd, disk_size_t *size)
{
  if(hnd->driver == NULL) {
    return DISK_ERROR_NOT_OPEN;
  }

  return DISK_DRIVER_FUNC_GET_SIZE(hnd->driver)(hnd->unit, size);
}

u08 disk_get_geo(disk_handle_t *hnd, disk_geo_t *geo)
{
  if(hnd->driver == NULL) {
    return DISK_ERROR_NOT_OPEN;
  }

  return DISK_DRIVER_FUNC_GET_GEO(hnd->driver)(hnd->unit, geo);
}

u08 disk_read(disk_handle_t *hnd, u32 lba, u16 num_blocks, u08 *data)
{
  if(hnd->driver == NULL) {
    return DISK_ERROR_NOT_OPEN;
  }

  return DISK_DRIVER_FUNC_READ(hnd->driver)(hnd->unit, lba, num_blocks, data);
}

u08 disk_write(disk_handle_t *hnd, u32 lba, u16 num_blocks, u08 *data)
{
  if(hnd->driver == NULL) {
    return DISK_ERROR_NOT_OPEN;
  }

  return DISK_DRIVER_FUNC_WRITE(hnd->driver)(hnd->unit, lba, num_blocks, data);
}
