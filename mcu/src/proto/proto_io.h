#ifndef PROTO_IO_H
#define PROTO_IO_H

extern void proto_io_init(void);
extern void proto_io_handle_cmd(void);

extern void proto_io_event_mask_add_chn(u08 chn);

// ----- API -----
extern u16  proto_io_api_get_default_mtu(void);
extern u16  proto_io_api_get_max_channels(void);
extern u16  proto_io_api_get_channel_mtu(u08 chn);
extern void proto_io_api_set_channel_mtu(u08 chn, u16 mtu);

extern void proto_io_api_open(u08 chn, u16 port);
extern void proto_io_api_close(u08 chn);
extern u16  proto_io_api_status(u08 chn);
extern void proto_io_api_reset(u08 chn);
extern void proto_io_api_seek(u08 chn, u32 off);
extern u32  proto_io_api_tell(u08 chn);

extern void proto_io_api_read_req(u08 chn, u16 size);
extern u16  proto_io_api_read_res(u08 chn);
extern void proto_io_api_read_blk(u08 chn, u16 *size, u08 **buf);
extern void proto_io_api_read_done(u08 chn, u16 size, u08 *buf);

extern void proto_io_api_write_req(u08 chn, u16 size);
extern u16  proto_io_api_write_res(u08 chn);
extern void proto_io_api_write_blk(u08 chn, u16 *size, u08 **buf);
extern void proto_io_api_write_done(u08 chn, u16 size, u08 *buf);

#endif
