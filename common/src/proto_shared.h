// proto_shared.h
// shared defines for parbox protocol

// registers
#define PROTO_MAX_RW_REG          32
#define PROTO_MAX_RO_REG          32
#define PROTO_MAX_CHANNEL         16

// command codes
#define PROTO_CMD_IDLE            0x00
#define PROTO_CMD_PING            0x10
#define PROTO_CMD_BOOTLOADER      0x11
#define PROTO_CMD_RESET           0x1f
#define PROTO_CMD_MSG_WRITE       0x20
#define PROTO_CMD_MSG_READ        0x30
#define PROTO_CMD_RW_REG_WRITE    0x40
#define PROTO_CMD_RW_REG_READ     0x60
#define PROTO_CMD_RO_REG_READ     0x80
#define PROTO_CMD_INVALID         0xff

#define PROTO_CMD_MASK            0xf0
