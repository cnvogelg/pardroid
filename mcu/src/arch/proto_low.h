#ifndef PROTO_LOW_H

extern void proto_low_init(void);
extern u08 proto_low_get_cmd(void);

extern void proto_low_no_value(void);

extern void proto_low_read_word(u16 v);
extern u16 proto_low_write_word(void);

extern u16 proto_low_write_block(u16 max_words, u08 *buffer);
extern void proto_low_read_block(u16 num_words, u08 *buffer);

extern void proto_low_ack_lo(void);
extern void proto_low_ack_hi(void);

extern void proto_low_pend_lo(void);
extern void proto_low_pend_hi(void);

#endif
