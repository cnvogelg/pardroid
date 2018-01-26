#ifndef STROBE_H
#define STROBE_H

#define STROBE_MAGIC_BYTE_HI       0xf1
#define STROBE_MAGIC_BYTE_LO       0xf2
#define STROBE_MAGIC_BYTE_EXIT     0xf3

#define STROBE_FLAG_NONE           0
#define STROBE_FLAG_GOT_STROBE     1
#define STROBE_FLAG_ALL_SENT       2
#define STROBE_FLAG_BUFFER_FILLED  4
#define STROBE_FLAG_IS_BUSY        8

extern void strobe_init(void);
extern void strobe_exit(void);
extern u08  strobe_get_key(u32 *key);
extern u08  strobe_get_data(void);

extern void strobe_send_begin(rom_pchar data, u16 size);
extern void strobe_send_end(void);

extern u08  strobe_read_flag(void);
extern void strobe_pulse_ack(void);

#endif
