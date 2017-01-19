#ifndef PROTO_LOW_H

extern void proto_low_init(void);
extern u08 proto_low_get_cmd(void);
extern void proto_low_ping(void);
extern void proto_low_test_read(u16 val);
extern u16 proto_low_test_write(void);

#endif
