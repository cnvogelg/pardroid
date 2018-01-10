#include "autoconf.h"
#include "types.h"
#include "arch.h"
#include "system.h"
#include "driver.h"

void driver_reset(u08 num)
{
  for(u08 did=0;did<num;did++) {
    driver_ptr_t drv = driver_get_quick(did);
    drv_reset_func_t f = (drv_reset_func_t)read_rom_rom_ptr(&drv->reset_func);
    if(f != 0) {
      f(did);
    }
  }
}

void driver_init(u08 num)
{
  for(u08 did=0;did<num;did++) {
    driver_ptr_t drv = driver_get_quick(did);
    driver_data_t *data = DRIVER_GET_DATA(did);
    drv_init_func_t f = (drv_init_func_t)read_rom_rom_ptr(&drv->init_func);
    if(f != 0) {
      if(f(did) == DRIVER_OK) {
        data->open_slots = 0;
      } else {
        data->open_slots = DRIVER_NO_INIT;
      }
    } else {
      data->open_slots = 0;
    }
  }
}

void driver_work(u08 num)
{
  for(u08 did=0;did<num;did++) {
    driver_ptr_t drv = driver_get_quick(did);
    driver_data_t *data = DRIVER_GET_DATA(did);
    drv_work_func_t f = (drv_work_func_t)read_rom_rom_ptr(&drv->work_func);
    if(f != 0) {
      f(did, data);
    }
  }
}

u08 driver_open(u08 did, u08 slot)
{
  driver_ptr_t drv = driver_get(did);
  if(drv != 0) {
    driver_data_t *data = DRIVER_GET_DATA(did);

    // was init successfull?
    if(data->open_slots == DRIVER_NO_INIT) {
      return DRIVER_ERROR_INIT_FAILED;
    }

    // check slot
    u08 num_slots = driver_get_num_slots(did);
    if(slot >= num_slots) {
      return DRIVER_ERROR_WRONG_SLOT;
    }

    // is already open?
    u08 mask = 1 << slot;
    if((data->open_slots & mask) == mask) {
      return DRIVER_ERROR_ALREADY_OPEN;
    }

    // ok to open
    drv_open_func_t f = (drv_open_func_t)read_rom_rom_ptr(&drv->open_func);
    u08 ok = DRIVER_OK;
    if(f != 0) {
      ok = f(did, slot);
    }
    if(ok == DRIVER_OK) {
      data->open_slots |= mask;
    }
    return ok;
  } else {
    return DRIVER_ERROR_NO_DRIVER;
  }
}

u08 driver_close(u08 did, u08 slot)
{
  driver_ptr_t drv = driver_get(did);
  if(drv != 0) {
    driver_data_t *data = DRIVER_GET_DATA(did);

    // check slot
    u08 num_slots = driver_get_num_slots(did);
    if(slot >= num_slots) {
      return DRIVER_ERROR_WRONG_SLOT;
    }

    // is really open?
    u08 mask = 1 << slot;
    if((data->open_slots & mask) == 0) {
      return DRIVER_ERROR_NOT_OPEN;
    }

    // close func (if any)
    drv_close_func_t f = (drv_close_func_t)read_rom_rom_ptr(&drv->close_func);
    if(f != 0) {
      f(did, slot);
    }

    // clear mask
    data->open_slots &= ~mask;
    return DRIVER_OK;
  } else {
    return DRIVER_ERROR_NO_DRIVER;
  }
}
