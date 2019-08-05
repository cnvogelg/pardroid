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

// -- actions --
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

// -- word function -- 
// read
#define PROTO_WFUNC_READ_FW_ID            0x00
#define PROTO_WFUNC_READ_FW_VERSION       0x01
#define PROTO_WFUNC_READ_MACHTAG          0x02
#define PROTO_WFUNC_READ_USER             0x03

// write
#define PROTO_WFUNC_WRITE_USER            0x00

// bootloader: read add ons
#define PROTO_WFUNC_READ_BOOT_ROM_FW_ID       (PROTO_WFUNC_READ_USER + 0)
#define PROTO_WFUNC_READ_BOOT_ROM_FW_VERSION  (PROTO_WFUNC_READ_USER + 1)
#define PROTO_WFUNC_READ_BOOT_ROM_MACHTAG     (PROTO_WFUNC_READ_USER + 2)
#define PROTO_WFUNC_READ_BOOT_ROM_CRC         (PROTO_WFUNC_READ_USER + 3)

// -- long function --
// read
#define PROTO_LFUNC_READ_STATUS           0x00
#define PROTO_LFUNC_READ_USER             0x01

// write
#define PROTO_LFUNC_WRITE_USER            0x00

// bootloader: read add ons
#define PROTO_LFUNC_READ_BOOT_ROM_SIZE        (PROTO_LFUNC_READ_USER + 0)

// -- status flag --
#define PROTO_STATUS_MASK_RX_PENDING  0x007f
#define PROTO_STATUS_MASK_ERROR       0x7f00
#define PROTO_STATUS_MASK_BUSY        0x0080
//#define PROTO_STATUS_MASK_FREE        0x8000
