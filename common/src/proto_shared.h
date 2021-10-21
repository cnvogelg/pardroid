// proto_shared.h
// shared defines for parbox protocol

#ifndef PROTO_SHARED_H
#define PROTO_SHARED_H

// number of channels
#define PROTO_MAX_CHANNEL         16

// command codes: upper 4 Bits of command word
#define PROTO_CMD_MASK            0xf0
#define PROTO_CMD_ARG             0x0f

// -- command table --
/* command bytes range from 0x00-0xff */

// masks for command ranges
#define PROTO_CMD_MASK_DEVICE         0x10
#define PROTO_CMD_MASK_WORD_VALUES    0x20
#define PROTO_CMD_MASK_LONG_VALUES    0x30

// 0x10 ... 0x1f  device actions
#define PROTO_CMD_DEVICE_PING         0x10
#define PROTO_CMD_DEVICE_BOOTLOADER   0x11
#define PROTO_CMD_DEVICE_RESET        0x12
#define PROTO_CMD_DEVICE_KNOK         0x13

// 0x20 ... 0x27  read word values
#define PROTO_CMD_RWORD_FW_ID         0x20
#define PROTO_CMD_RWORD_FW_VERSION    0x21
#define PROTO_CMD_RWORD_MACH_TAG      0x22
#define PROTO_CMD_RWORD_MAX_MTU       0x23
#define PROTO_CMD_RWORD_MAX_CHANNELS  0x24
#define PROTO_CMD_RWORD_EVENT_MASK    0x25
#define PROTO_CMD_RWORD_DRIVER_TOKEN  0x26

// 0x28 ... 0x2f  write word values
#define PROTO_CMD_WWORD_CHANNEL_ID    0x28
#define PROTO_CMD_WWORD_CHANNEL_MTU   0x29
#define PROTO_CMD_WWORD_DRIVER_TOKEN  0x2a

// 0x30 ... 0x37  read long - UNUSED
// 0x38 ... 0x3f  write long - UNUSED

// --- channel commands ---
// lower nybble is always the channel id 0..15
// upper nybble is command
// channel commands are located in 0x40..0xff
#define PROTO_CMD_CHANNEL             0x40

// open: write word with port
#define PROTO_CMD_CHANNEL_OPEN        0x40
// close: action
#define PROTO_CMD_CHANNEL_CLOSE       0x50
// read_req: write word with max size
#define PROTO_CMD_CHANNEL_READ_REQ    0x60
// read_size: read word with actual size
#define PROTO_CMD_CHANNEL_READ_SIZE   0x70
// read: read block
#define PROTO_CMD_CHANNEL_READ        0x80
// write_req: write word with size
#define PROTO_CMD_CHANNEL_WRITE_REQ   0x90
// write: write block
#define PROTO_CMD_CHANNEL_WRITE       0xa0
// seek: write long with offset
#define PROTO_CMD_CHANNEL_SEEK        0xb0
// reset: action
#define PROTO_CMD_CHANNEL_RESET       0xc0
// status: read word
#define PROTO_CMD_CHANNEL_STATUS      0xd0

// ----- channel status mask word -----
// lower byte is bit field with status bits
// upper byte is error code (!=0)
#define PROTO_STATUS_MASK_BITS        0x0f
#define PROTO_STATUS_MASK_ERROR       0xf0
// -- bits --
// open: channel is open
#define PROTO_STATUS_BITS_OPEN        0x01
// read pending: a read request is ready to be retrieved
#define PROTO_STATUS_BITS_READ_PEND   0x02
// read size: a read size != req size needs to be retrieved
#define PROTO_STATUS_BITS_READ_SIZE   0x04
// write pending: a write request is ready to be sent
#define PROTO_STATUS_BITS_WRITE_PEND  0x08
// end of stream reached
#define PROTO_STATUS_BITS_EOS         0x10
// stream was reset
#define PROTO_STATUS_BITS_RESET       0x20

#endif
