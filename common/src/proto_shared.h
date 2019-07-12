// proto_shared.h
// shared defines for parbox protocol

// number of channels
#define PROTO_MAX_CHANNEL         16
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
#define PROTO_CMD_MSG_READ_DATA   0x60
#define PROTO_CMD_MSG_WRITE_DATA  0x70
#define PROTO_CMD_MSG_READ_SIZE   0x80
#define PROTO_CMD_MSG_WRITE_SIZE  0x90

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
#define PROTO_WFUNC_CHAN_RX_PEND  0x01
#define PROTO_WFUNC_CHAN_ERROR    0x02
#define PROTO_WFUNC_REG_ADDR      0x03
#define PROTO_WFUNC_REG_VALUE     0x04
#define PROTO_WFUNC_USER          0x05

// word function numbers 0..15: bootloader
#define PROTO_WFUNC_BOOT_MAGIC        0x00
#define PROTO_WFUNC_BOOT_MACHTAG      0x01
#define PROTO_WFUNC_BOOT_VERSION      0x02
#define PROTO_WFUNC_BOOT_PAGE_SIZE    0x03
#define PROTO_WFUNC_BOOT_ROM_CRC      0x04
#define PROTO_WFUNC_BOOT_ROM_MACHTAG  0x05
#define PROTO_WFUNC_BOOT_ROM_FW_VERSION  0x06
#define PROTO_WFUNC_BOOT_ROM_FW_ID    0x07

// long function numbers 0..15: application
#define PROTO_LFUNC_USER              0x00

// long function numbers 0..15: bootloader
#define PROTO_LFUNC_BOOT_ROM_SIZE     0x00
#define PROTO_LFUNC_BOOT_PAGE_ADDR    0x01

// registers
#define PROTO_REG_RANGE_MASK      0xff00
#define PROTO_REG_REGISTER_MASK   0x00ff

// register ranges
#define PROTO_REG_RANGE_GLOBAL    0x10

// global registers
#define PROTO_REG_GLOBAL_MACHTAG      0x00
#define PROTO_REG_GLOBAL_FW_ID        0x01
#define PROTO_REG_GLOBAL_FW_VERSION   0x02
