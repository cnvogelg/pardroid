#ifndef UTIL_H
#define UTIL_H

extern void util_put_uword(u16 val, u08 *buf);
extern u16  util_get_uword(u08 *buf);

extern void util_put_ulong(u32 val, u08 *buf);
extern u32  util_get_ulong(u08 *buf);

#endif
