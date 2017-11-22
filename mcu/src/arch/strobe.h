#ifndef STROBE_H
#define STROBE_H

#define STROBE_MAGIC_BYTE_HI       0xf1
#define STROBE_MAGIC_BYTE_LO       0xf2
#define STROBE_MAGIC_BYTE_EXIT     0xf3

extern void strobe_init(void);
extern void strobe_exit(void);
extern u08  strobe_get_key(u32 *key);

#endif
