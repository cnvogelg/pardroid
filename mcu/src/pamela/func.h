#ifndef FUNC_H
#define FUNC_H

extern u16 wfunc_read_handle(u08 num);
extern void wfunc_write_handle(u08 num, u16 val);
extern u32 lfunc_read_handle(u08 num);
extern void lfunc_write_handle(u08 num, u32 val);

// ----- API ----
extern u16  proto_api_reg_read(u16 addr);
extern void proto_api_reg_write(u16 addr, u16 val);

#endif
