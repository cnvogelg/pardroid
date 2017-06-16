// proto_shared.h
// shared defines for parbox protocol

// number of channels
#define PROTO_MAX_CHANNEL         8
#define PROTO_MAX_FUNCTION        8
#define PROTO_MAX_ACTION          8

// command codes: bit 4+3 in idle byte
#define PROTO_CMD_MASK            0x18
#define PROTO_CMD_FULL_MASK       0x1f
#define PROTO_CMD_SUB_MASK        0x07
#define PROTO_CMD_ACTION          0x00
#define PROTO_CMD_MSG_READ        0x08
#define PROTO_CMD_MSG_WRITE       0x10
#define PROTO_CMD_FUNCTION        0x18

// actions
#define PROTO_ACTION_IDLE         0x00
#define PROTO_ACTION_PING         0x01
#define PROTO_ACTION_BOOTLOADER   0x02
#define PROTO_ACTION_RESET        0x03
#define PROTO_ACTION_USER         0x04

// functions
#define PROTO_FUNC_REGADDR_GET    0x00
#define PROTO_FUNC_REGADDR_SET    0x01
#define PROTO_FUNC_REG_READ       0x02
#define PROTO_FUNC_REG_WRITE      0x03
#define PROTO_FUNC_GET_ERROR      0x04
#define PROTO_FUNC_USER           0x05

// device status: bit 7,6,5 in idle byte (set by device)
#define PROTO_STATUS_MASK         0xe0
#define PROTO_STATUS_OK           0x00
#define PROTO_STATUS_BOOTLOADER   0x20
#define PROTO_STATUS_DETACHED     0x40
#define PROTO_STATUS_ERROR        0x80
#define PROTO_STATUS_READ_PENDING 0x10 // is or'ed in from pending line

// register definitions
#define PROTO_REG_FW_VERSION        0
#define PROTO_REG_FW_MACHTAG        1
#define PROTO_REG_FW_ID             2
#define PROTO_REG_NUM_REGS          3
#define PROTO_REG_PEND_TOTAL        4
#define PROTO_REG_USER              5

// firmware ids
#define PROTO_FWID_TEST           1
