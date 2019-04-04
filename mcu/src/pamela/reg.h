#ifndef REG_H
#define REG_H

extern void reg_set_addr(u16 addr);
extern u16 reg_get_addr(void);

extern void reg_set_value(u16 addr);
extern u16  reg_get_value(void);

extern u16  reg_wfunc_read_handle(u08 num);
extern void reg_wfunc_write_handle(u08 num, u16 val);

// implement this
extern void reg_api_set_value(u08 range, u08 reg, u16 value);
extern u16 reg_api_get_value(u08 range, u08 reg);

#endif
