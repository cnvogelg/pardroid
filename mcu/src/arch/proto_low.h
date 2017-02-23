#ifndef PROTO_LOW_H

extern void proto_low_init(void);
extern u08 proto_low_get_cmd(void);

extern void proto_low_ping(void);

extern void proto_low_reg_read(u16 v);
extern u16 proto_low_reg_write(void);

extern u16 proto_low_msg_write(u16 max_words, u08 *buffer);
extern void proto_low_msg_read(u16 num_words, u08 *buffer);

#endif
