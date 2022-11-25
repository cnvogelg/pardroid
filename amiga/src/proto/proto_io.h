#ifndef PROTO_IO_H
#define PROTO_IO_H

#include "proto_dev.h"

typedef UWORD channel_t;
typedef LONG  offset_t;
typedef UWORD io_size_t;

/* init/exit of io mode */
extern proto_handle_t *proto_io_init(proto_env_handle_t *penv);
extern void proto_io_exit(proto_handle_t *ph);

/* global config */
extern int proto_io_get_event_mask(proto_handle_t *ph, UWORD *events);
extern int proto_io_get_default_mtu(proto_handle_t *ph, UWORD *mtu);
extern int proto_io_get_max_channels(proto_handle_t *ph, UWORD *channels);

/* per channel config */
extern int proto_io_get_channel_mtu(proto_handle_t *ph, channel_t ch, UWORD *mtu);
extern int proto_io_set_channel_mtu(proto_handle_t *ph, channel_t ch, UWORD mtu);
extern int proto_io_get_channel_error(proto_handle_t *ph, channel_t chn, UWORD *error);

/* channel commands */
extern int proto_io_open(proto_handle_t *ph, channel_t ch, UWORD port);
extern int proto_io_close(proto_handle_t *ph, channel_t ch);
extern int proto_io_status(proto_handle_t *ph, channel_t ch, UWORD *status);
extern int proto_io_reset(proto_handle_t *ph, channel_t ch);
extern int proto_io_seek(proto_handle_t *ph, channel_t ch, offset_t offset);
extern int proto_io_tell(proto_handle_t *ph, channel_t ch, offset_t *offset);
/* read */
extern int proto_io_read_request(proto_handle_t *ph, channel_t ch, io_size_t size);
extern int proto_io_read_result(proto_handle_t *ph, channel_t ch, io_size_t *size);
extern int proto_io_read_data(proto_handle_t *ph, channel_t ch, UBYTE *data, io_size_t size);
/* write */
extern int proto_io_write_request(proto_handle_t *ph, channel_t ch, io_size_t size);
extern int proto_io_write_result(proto_handle_t *ph, channel_t ch, io_size_t *size);
extern int proto_io_write_data(proto_handle_t *ph, channel_t ch, UBYTE *data, io_size_t size);

#endif
