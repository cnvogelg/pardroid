/* channel IO command set */
#ifndef PROTO_IO_SHARED_H
#define PROTO_IO_SHARED_H

/* the lower nybble of a command selects the channel */
#define PROTO_IO_NUM_CHANNELS             16
#define PROTO_IO_CHANNEL_MASK             0x0f
#define PROTO_IO_CMD_MASK                 0xf0

/* ----- global range 0x20..0x2f ----- */
#define PROTO_IO_CMD_GLOBAL_MASK          0x20
/* read event mask: one bit per channel that has status updates */
#define PROTO_IO_CMD_RWORD_EVENT_MASK     0x20
/* global: get default mtu size */
#define PROTO_IO_CMD_RWORD_DEFAULT_MTU    0x21
/* global: get supported number of channels */
#define PROTO_IO_CMD_RWORD_MAX_CHANNELS   0x22

/* ----- multiplexed channel commands -----*/
/* set a channel number for the following commands */
#define PROTO_IO_CMD_WWORD_CHANNEL_NO     0x27
/* channel: get current mtu */
#define PROTO_IO_CMD_RWORD_CHANNEL_MTU    0x28
/* channel: try to set mtu */
#define PROTO_IO_CMD_WWORD_CHANNEL_MTU    0x29

/* ----- channel commands (lower nybble is channel no) ----- */
/* open a channel and pass port number */
#define PROTO_IO_CMD_WWORD_OPEN           0x30
/* close a channel */
#define PROTO_IO_CMD_ACTION_CLOSE         0x40
/* channel status and return status word */
#define PROTO_IO_CMD_RWORD_STATUS         0x50
/* reset channel */
#define PROTO_IO_CMD_ACTION_RESET         0x60
/* seek to long offset */
#define PROTO_IO_CMD_WLONG_SEEK           0x70
/* tell seek offset */
#define PROTO_IO_CMD_RLONG_TELL           0x80

/* read request with max size in bytes */
#define PROTO_IO_CMD_WWORD_READ_REQ       0x90
/* return actual read size (if not req size) */
#define PROTO_IO_CMD_RWORD_READ_RESULT    0xa0
/* read data of action size */
#define PROTO_IO_CMD_RBLOCK_READ_DATA     0xb0

/* write request with size in bytes */
#define PROTO_IO_CMD_WWORD_WRITE_REQ      0xc0
/* return actual write size (if not req size) */
#define PROTO_IO_CMD_RWORD_WRITE_RESULT   0xd0
/* write data */
#define PROTO_IO_CMD_WBLOCK_WRITE_DATA    0xe0

/* ----- status word ----- */
// open: channel is open
#define PROTO_IO_STATUS_OPEN        0x01
// end of stream reached
#define PROTO_IO_STATUS_EOS         0x02
// stream has error
#define PROTO_IO_STATUS_ERROR       0x04

// read pending: a read request is ready to be retrieved
#define PROTO_IO_STATUS_READ_PEND   0x10
// read size: a read size != req size needs to be retrieved
#define PROTO_IO_STATUS_READ_SIZE   0x20
// read requested: a read request was posted
#define PROTO_IO_STATUS_READ_REQ    0x40
// read requested was aborted by device
#define PROTO_IO_STATUS_READ_ERROR  0x80
// mask for all read bits
#define PROTO_IO_STATUS_READ_MASK   0xF0

// write pending: a write request is ready to be sent
#define PROTO_IO_STATUS_WRITE_PEND  0x100
// write size: a write size != req size needs to be retrieved
#define PROTO_IO_STATUS_WRITE_SIZE  0x200
// write requested: a write request was posted
#define PROTO_IO_STATUS_WRITE_REQ   0x400
// write requested was aborted by device
#define PROTO_IO_STATUS_WRITE_ERROR 0x800
// mask for all read bits
#define PROTO_IO_STATUS_WRITE_MASK  0xF00

/* ----- error codes ----- */
#define PROTO_IO_OK                 0
#define PROTO_IO_ERROR_ALREADY_OPEN 1
#define PROTO_IO_ERROR_NOT_OPEN     2
#define PROTO_IO_ERROR_DUP_READ     3
#define PROTO_IO_ERROR_DUP_WRITE    4

#endif
