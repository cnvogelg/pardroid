#ifndef WIZ_REGS_H
#define WIZ_REGS_H

extern void wiz_reg_base_write(u16 addr, u08 value);
extern void wiz_reg_base_write_buf(u16 addr, const u08 *buf, u16 len);
extern u08  wiz_reg_base_read(u16 addr);
extern void wiz_reg_base_read_buf(u16 addr, u08 *buf, u16 len);


#endif
