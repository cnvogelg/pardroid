// proto_shared.h
// shared defines for parbox protocol

// number of channels
#define PROTO_MAX_CHANNEL         8
#define PROTO_MAX_FUNCTION        16
#define PROTO_MAX_ACTION          16

// command codes: upper 4 Bits of command word
#define PROTO_CMD_MASK            0xf0
#define PROTO_CMD_ARG             0x0f

#define PROTO_CMD_ACTION          0x10
#define PROTO_CMD_WFUNC_READ      0x20
#define PROTO_CMD_WFUNC_WRITE     0x30
#define PROTO_CMD_LFUNC_READ      0x40
#define PROTO_CMD_LFUNC_WRITE     0x50
#define PROTO_CMD_MSG_READ        0x60
#define PROTO_CMD_MSG_WRITE       0x70

// actions
#define PROTO_ACTION_PING         0x00
#define PROTO_ACTION_BOOTLOADER   0x01
#define PROTO_ACTION_RESET        0x02
#define PROTO_ACTION_USER         0x03

// combined actions
#define PROTO_CMD_ACTION_PING       0x10
#define PROTO_CMD_ACTION_BOOTLOADER 0x11
#define PROTO_CMD_ACTION_RESET      0x12

// word function numbers 0..15
#define PROTO_WFUNC_REG_ADDR      0x00
#define PROTO_WFUNC_REG_VALUE     0x01
#define PROTO_WFUNC_USER          0x02

// long function numbers 0..15
#define PROTO_LFUNC_USER          0x00
