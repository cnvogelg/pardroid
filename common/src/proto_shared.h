// proto_shared.h
// shared defines for parbox protocol

// registers
#define PROTO_MAX_RW_REG          32
#define PROTO_MAX_RO_REG          32
#define PROTO_MAX_CHANNEL         16

// command codes
#define PROTO_CMD_IDLE            0x00

#define PROTO_CMD_ACTION          0x10
#define PROTO_ACTION_PING         0x10
#define PROTO_ACTION_BOOTLOADER   0x11
#define PROTO_ACTION_RESET        0x12
#define PROTO_ACTION_USER         0x13

#define PROTO_CMD_MSG_WRITE       0x20
#define PROTO_CMD_MSG_READ        0x30
#define PROTO_CMD_RW_REG_WRITE    0x40
#define PROTO_CMD_RW_REG_READ     0x60
#define PROTO_CMD_RO_REG_READ     0x80
#define PROTO_CMD_INVALID         0xff

#define PROTO_CMD_MASK            0xf0

// ro register definitions
#define PROTO_RO_REG_VERSION      0
#define PROTO_RO_REG_MACHTAG      1
#define PROTO_RO_REG_FW_ID        2
#define PROTO_RO_REG_NUM_RO_RW    3
#define PROTO_RO_REG_PEND_MASK    4
#define PROTO_RO_REG_PEND_TOTAL   5

#define PROTO_RO_REG_USER         6

// firmware ids
#define PROTO_FWID_TEST           1
