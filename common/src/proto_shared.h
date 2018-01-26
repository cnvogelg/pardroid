// proto_shared.h
// shared defines for parbox protocol

// number of channels
#define PROTO_MAX_CHANNEL         8
#define PROTO_MAX_FUNCTION        8
#define PROTO_MAX_ACTION          8

// command codes: bits 0..3 in idle byte + cflag line (SEL) or'ed in as bit 4
#define PROTO_CMD_MASK            0x18
#define PROTO_CMD_FULL_MASK       0x1f
#define PROTO_CMD_SUB_MASK        0x07
#define PROTO_CMD_ACTION          0x10
#define PROTO_CMD_MSG_READ        0x08
#define PROTO_CMD_MSG_WRITE       0x00
#define PROTO_CMD_FUNCTION        0x18

// actions
#define PROTO_ACTION_PING         0x00
#define PROTO_ACTION_DELAY_RESET  0x01
#define PROTO_ACTION_BOOTLOADER   0x02
#define PROTO_ACTION_RESET        0x03
#define PROTO_ACTION_ATTACH       0x04
#define PROTO_ACTION_DETACH       0x05

// functions
#define PROTO_FUNC_REGADDR_GET    0x00
#define PROTO_FUNC_REGADDR_SET    0x01
#define PROTO_FUNC_REG_READ       0x02
#define PROTO_FUNC_REG_WRITE      0x03
#define PROTO_FUNC_OFFSLOT_GET    0x04
#define PROTO_FUNC_OFFSLOT_SET    0x05
#define PROTO_FUNC_OFFSET_GET     0x06
#define PROTO_FUNC_OFFSET_SET     0x07

// device status: bit 7,6,5 in idle byte (set by device)
#define PROTO_STATUS_MASK         0xf0
#define PROTO_STATUS_INIT         0x00
#define PROTO_STATUS_BOOTLOADER   0x10
#define PROTO_STATUS_ATTACHED     0x20
#define PROTO_STATUS_EVENTS       0x40
#define PROTO_STATUS_READ_PENDING 0x80
#define PROTO_STATUS_CHANNEL_MASK 0x70

// register definitions
#define PROTO_REGOFFSET_BASE             0x00
#define PROTO_REGOFFSET_HANDLER          0x20
#define PROTO_REGOFFSET_USER             0x80
