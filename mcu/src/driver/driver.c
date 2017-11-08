#include "types.h"
#include "arch.h"
#include "system.h"
#include "driver.h"

static driver_ptr_t get_driver(u08 num)
{
  return (driver_ptr_t)read_rom_rom_ptr(&driver_table[num]);
}

void driver_init(u08 num)
{
  for(u08 did=0;did<num;did++) {
    driver_ptr_t drv = get_driver(did);
    driver_data_t *data = DRIVER_GET_DATA(did);
    drv_init_func_t f = (drv_init_func_t)read_rom_rom_ptr(&drv->init_func);
    if(f != 0) {
      if(f(did)) {
        data->flags = DRIVER_FLAG_INIT;
      } else {
        data->flags = 0;
      }
    } else {
      data->flags = DRIVER_FLAG_INIT;
    }
  }
}

void driver_work(u08 num)
{
  for(u08 did=0;did<num;did++) {
    driver_ptr_t drv = get_driver(did);
    driver_data_t *data = DRIVER_GET_DATA(did);
    drv_work_func_t f = (drv_work_func_t)read_rom_rom_ptr(&drv->work_func);
    if(f != 0) {
      f(did, data->flags);
    }
  }
}

u08 driver_open(u08 did)
{
  u08 max = read_rom_char(&driver_table_size);
  if(did < max) {
    driver_ptr_t hnd = get_driver(did);
    drv_open_func_t f = (drv_open_func_t)read_rom_rom_ptr(&hnd->open_func);
    if(f != 0) {
      return f(did);
    } else {
      return DRIVER_OK;
    }
  } else {
    return DRIVER_ERROR_INDEX;
  }
}

void driver_close(u08 did)
{
  u08 max = read_rom_char(&driver_table_size);
  if(did < max) {
    driver_ptr_t hnd = get_driver(did);
    drv_close_func_t f = (drv_close_func_t)read_rom_rom_ptr(&hnd->close_func);
    if(f != 0) {
      return f(did);
    }
  }
}

u16 driver_read(u08 did, u08 *buf, u16 size)
{
  u08 max = read_rom_char(&driver_table_size);
  if(did < max) {
    driver_ptr_t hnd = get_driver(did);
    drv_read_func_t f = (drv_read_func_t)read_rom_rom_ptr(&hnd->read_func);
    return f(did, buf, size);
  } else {
    return 0;
  }
}

u16 driver_write(u08 did, u08 *buf, u16 size)
{
  u08 max = read_rom_char(&driver_table_size);
  if(did < max) {
    driver_ptr_t hnd = get_driver(did);
    drv_write_func_t f = (drv_write_func_t)read_rom_rom_ptr(&hnd->write_func);
    return f(did, buf, size);
  } else {
    return 0;
  }
}

void driver_get_mtu(u08 did, u16 *mtu_max, u16 *mtu_min)
{
  u08 max = read_rom_char(&driver_table_size);
  if(did < max) {
    driver_ptr_t hnd = get_driver(did);
    *mtu_max = read_rom_word(&hnd->mtu_max);
    *mtu_min = read_rom_word(&hnd->mtu_min);
  } else {
    *mtu_max = 0;
    *mtu_min = 0;
  }
}
