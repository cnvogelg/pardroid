#ifndef I2C_H
#define I2C_H

extern void i2c_init(void);
extern u08 i2c_write(u08 addr, const u08 *data, u16 len);
extern u08 i2c_read(u08 addr, u08 *data, u16 len);

extern u08 i2c_start(u08 addr, u08 write);
extern void i2c_stop(void);
extern u08 i2c_write_byte(u08 data);
extern u08 i2c_read_byte(u08 ack);

#endif
