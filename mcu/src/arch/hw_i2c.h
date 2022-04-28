#ifndef HW_I2C_H
#define HW_I2C_H

#include "arch.h"

#define HW_I2C_OK       0
#define HW_I2C_ERROR    1

extern void hw_i2c_init(void);
extern u08 hw_i2c_write(u08 addr, const u08 *data, u16 len);
extern u08 hw_i2c_write_rom(u08 addr, rom_pchar data, u16 len);
extern u08 hw_i2c_read(u08 addr, u08 *data, u16 len);

#endif
