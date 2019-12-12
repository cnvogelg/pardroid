#ifndef PROTO_API_H
#define PROTO_API_H

// action handler
extern void proto_api_action(u08 num);

// func read/write
extern u16  proto_api_wfunc_read(u08 num);
extern void proto_api_wfunc_write(u08 num, u16 val);
extern u32  proto_api_lfunc_read(u08 num);
extern void proto_api_lfunc_write(u08 num, u32 val);

// read message
extern u08 *proto_api_read_msg_begin(u08 chan, u16 *size);
extern void proto_api_read_msg_done(u08 chan);

// write message
extern u08 *proto_api_write_msg_begin(u08 chan, u16 *size);
extern void proto_api_write_msg_done(u08 chan);

// channel ops
extern void proto_api_chn_set_rx_offset(u08 chan, u32 offset);
extern void proto_api_chn_set_tx_offset(u08 chan, u32 offset);
extern u16  proto_api_chn_get_rx_size(u08 chan);
extern void proto_api_chn_set_rx_size(u08 chan, u16 size);
extern void proto_api_chn_set_tx_size(u08 chan, u16 size);
extern void proto_api_chn_request_rx(u08 chan);
extern void proto_api_chn_cancel_rx(u08 chan);
extern void proto_api_chn_cancel_tx(u08 chan);

#endif
