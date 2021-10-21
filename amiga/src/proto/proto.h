#ifndef PROTO_H
#define PROTO_H

#include "proto_shared.h"
#include "proto_ext.h"

// error codes
#define PROTO_RET_OK                0
#define PROTO_RET_RAK_INVALID       1
#define PROTO_RET_TIMEOUT           2
#define PROTO_RET_SLAVE_ERROR       3
#define PROTO_RET_MSG_TOO_LARGE     4
#define PROTO_RET_DEVICE_BUSY       5
#define PROTO_RET_INVALID_CHANNEL   15
#define PROTO_RET_INVALID_FUNCTION  14
#define PROTO_RET_INVALID_ACTION    13
#define PROTO_RET_INVALID_MTU       12

// handle
struct proto_handle;
typedef struct proto_handle proto_handle_t;

// init/exit of handle
extern proto_handle_t *proto_init(struct pario_port *port, struct timer_handle *th, struct Library *SysBase);
extern void proto_exit(proto_handle_t *ph);

// device commands
extern int proto_device_ping(proto_handle_t *ph);
extern int proto_device_reset(proto_handle_t *ph);
extern int proto_device_bootloader(proto_handle_t *ph);
extern int proto_device_knok(proto_handle_t *ph);

// read/write word values
extern int proto_read_word(proto_handle_t *ph, UBYTE num, UWORD *data);
extern int proto_write_word(proto_handle_t *ph, UBYTE num, UWORD data);

// read/write long values
extern int proto_read_long(proto_handle_t *ph, UBYTE num, ULONG *data);
extern int proto_write_long(proto_handle_t *ph, UBYTE num, ULONG data);

// channel commands
extern int proto_open(proto_handle_t *ph, UWORD port);
extern int proto_close(proto_handle_t *ph);
// read
extern int proto_read_req(proto_handle_t *ph, UBYTE chn, UWORD max_bytes);
extern int proto_read_size(proto_handle_t *ph, UWORD *num_bytes);
extern int proto_read(proto_handle_t *ph, UBYTE chn, UBYTE *buf, UWORD num_bytes);
// write
extern int proto_write_req(proto_handle_t *ph, UBYTE chn, UWORD num_bytes);
extern int proto_write(proto_handle_t *ph, UBYTE chn, UBYTE *buf, UWORD num_words);
// seek
extern int proto_seek(proto_handle_t *ph, UBYTE chn, ULONG offset);
// reset
extern int proto_reset(proto_handle_t *ph, UBYTE chn);
// status
extern int proto_status(proto_handle_t *ph, UBYTE chn, UWORD *status);

// read word values
int proto_get_fw_id(proto_handle_t *ph, UWORD *fw_id);
int proto_get_fw_version(proto_handle_t *ph, UWORD *fw_version);
int proto_get_mach_tag(proto_handle_t *ph, UWORD *mach_tag);
int proto_get_max_mtu(proto_handle_t *ph, UWORD *fw_id);
int proto_get_max_channels(proto_handle_t *ph, UWORD *fw_version);
int proto_get_event_mask(proto_handle_t *ph, UWORD *mach_tag);
int proto_get_driver_token(proto_handle_t *ph, UWORD *fw_id);

// write word values
int proto_set_channel_id(proto_handle_t *ph, UWORD id);
int proto_set_channel_mtu(proto_handle_t *ph, UWORD mtu);
int proto_set_driver_token(proto_handle_t *ph, UWORD token);

#endif
