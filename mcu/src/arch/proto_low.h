#ifndef PROTO_LOW_H

extern void proto_low_init(void);
extern u08 proto_low_get_cmd(void);

extern void proto_low_action(void);
extern void proto_low_end(void);

extern void proto_low_read_word(u16 v);
extern u16  proto_low_write_word(void);

extern void proto_low_read_long(u32 v);
extern u32  proto_low_write_long(void);

extern u16  proto_low_write_block(u16 max_words, u08 *buffer, u16 *crc);
extern void proto_low_read_block(u16 num_words, u08 *buffer, u16 crc);

extern void proto_low_ack_lo(void);
extern void proto_low_ack_hi(void);

#endif
