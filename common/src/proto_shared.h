// proto_shared.h
// shared defines for parbox protocol

// number of channels
#define PROTO_MAX_CHANNEL         15

#define PROTO_MAX_FUNCTION        16
#define PROTO_MAX_ACTION          16

#define PROTO_MTU_INVALID         0xffff

// command codes: upper 4 Bits of command word
#define PROTO_CMD_MASK            0xf0
#define PROTO_CMD_ARG             0x0f

#define PROTO_CMD_ACTION          0x10
#define PROTO_CMD_WFUNC_READ      0x20
#define PROTO_CMD_WFUNC_WRITE     0x30
#define PROTO_CMD_LFUNC_READ      0x40
#define PROTO_CMD_LFUNC_WRITE     0x50
#define PROTO_CMD_MSG_READ_DATA   0x60
#define PROTO_CMD_MSG_WRITE_DATA  0x70
#define PROTO_CMD_MSG_READ_SIZE   0x80
#define PROTO_CMD_MSG_WRITE_SIZE  0x90
#define PROTO_CMD_READ_OFFSET     0xa0
#define PROTO_CMD_WRITE_OFFSET    0xb0
#define PROTO_CMD_READ_MTU        0xc0
#define PROTO_CMD_WRITE_MTU       0xd0

// actions
#define PROTO_ACTION_PING         0x00
#define PROTO_ACTION_BOOTLOADER   0x01
#define PROTO_ACTION_RESET        0x02
#define PROTO_ACTION_KNOK         0x03
#define PROTO_ACTION_USER         0x04

// combined actions
#define PROTO_CMD_ACTION_PING       0x10
#define PROTO_CMD_ACTION_BOOTLOADER 0x11
#define PROTO_CMD_ACTION_RESET      0x12
#define PROTO_CMD_ACTION_KNOK       0x13

// magic type
#define PROTO_MAGIC_APPLICATION     0x4711
#define PROTO_MAGIC_BOOTLOADER      0x2342

// word function numbers 0..15: application
#define PROTO_WFUNC_MAGIC         0x00
#define PROTO_WFUNC_USER          0x01

// word function numbers 0..15: bootloader
#define PROTO_WFUNC_BOOT_MAGIC        0x00
#define PROTO_WFUNC_BOOT_MACHTAG      0x01
#define PROTO_WFUNC_BOOT_VERSION      0x02
#define PROTO_WFUNC_BOOT_ROM_CRC      0x03
#define PROTO_WFUNC_BOOT_ROM_MACHTAG  0x04
#define PROTO_WFUNC_BOOT_ROM_FW_VERSION  0x05
#define PROTO_WFUNC_BOOT_ROM_FW_ID    0x06

// long function numbers 0..15: application
#define PROTO_LFUNC_STATUS            0x00
#define PROTO_LFUNC_USER              0x01

// long function numbers 0..15: bootloader
#define PROTO_LFUNC_BOOT_ROM_SIZE     0x00


// status flag
#define PROTO_STATUS_MASK_RX_PENDING  0x007f
#define PROTO_STATUS_MASK_ERROR       0x7f00
#define PROTO_STATUS_MASK_BUSY        0x0080
//#define PROTO_STATUS_MASK_FREE        0x8000
