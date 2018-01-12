#ifndef PROTO_H

#include "proto_shared.h"

extern void proto_init(u08 status);
extern void proto_handle(void);
extern u08  proto_current_cmd(void);

// define these in your code
extern void proto_api_action(u08 num);
extern void proto_api_function(u08 num);

// high level message i/o api
extern u08 *proto_api_read_msg_prepare(u08 chan,u16 *size, u16 *extra);
extern void proto_api_read_msg_done(u08 chan, u08 status);
extern u08 *proto_api_write_msg_prepare(u08 chan,u16 *max_size);
extern void proto_api_write_msg_done(u08 chan,u16 size, u16 extra);

// low level message i/o api
extern void proto_api_read_msg(u08 chan);
extern void proto_api_write_msg(u08 chan);

extern u08  proto_api_get_end_status(void);

#endif
