#ifndef PROTO_H

#include "proto_shared.h"

extern void proto_init(void);
extern void proto_handle(void);

// define these in your code
extern void proto_api_action(u08 num);
extern void proto_api_function(u08 num);

extern u08 *proto_api_read_msg_prepare(u08 chan,u16 *size);
extern void proto_api_read_msg_done(u08 chan);
extern u08 *proto_api_write_msg_prepare(u08 chan,u16 *max_size);
extern void proto_api_write_msg_done(u08 chan,u16 size);

extern u08  proto_api_read_is_pending(void);

#endif
