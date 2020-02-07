// proto_shared.h
// shared defines for parbox protocol

#ifndef PROTO_SHARED_H
#define PROTO_SHARED_H

// number of channels
#define PROTO_MAX_CHANNEL         14

#define PROTO_MAX_FUNCTION        16
#define PROTO_MAX_ACTION          16

#define PROTO_MTU_INVALID         0xffff

// command codes: upper 4 Bits of command word
#define PROTO_CMD_MASK            0xf0
#define PROTO_CMD_ARG             0x0f

// -- command table --
/* global commands */
#define PROTO_CMD_ACTION                        0x10
#define PROTO_CMD_WFUNC_READ                    0x20
#define PROTO_CMD_WFUNC_WRITE                   0x30
#define PROTO_CMD_LFUNC_READ                    0x40
#define PROTO_CMD_LFUNC_WRITE                   0x50
// channel commands (first nibble is channel)
#define PROTO_CMD_CHN_READ_DATA                 0x60
#define PROTO_CMD_CHN_WRITE_DATA                0x70
// extended (non-mini) command set
#define PROTO_CMD_CHN_GET_RX_SIZE               0x80
#define PROTO_CMD_CHN_SET_TX_SIZE               0x90
#define PROTO_CMD_CHN_SET_OFFSET                0xa0
#define PROTO_CMD_CHN_CANCEL_TRANSFER           0xb0

// -- actions --
#define PROTO_ACTION_PING                       0x00
#define PROTO_ACTION_BOOTLOADER                 0x01
#define PROTO_ACTION_RESET                      0x02
#define PROTO_ACTION_KNOK                       0x03
// channel actions
#define PROTO_ACTION_CHN_OPEN                   0x04
#define PROTO_ACTION_CHN_CLOSE                  0x05
#define PROTO_ACTION_CHN_RESET                  0x06
#define PROTO_ACTION_USER                       0x07

// combined actions
#define PROTO_CMD_ACTION_PING                   0x10
#define PROTO_CMD_ACTION_BOOTLOADER             0x11
#define PROTO_CMD_ACTION_RESET                  0x12
#define PROTO_CMD_ACTION_KNOK                   0x13

// -- word function -- 
// read
#define PROTO_WFUNC_READ_FW_ID                  0x00
#define PROTO_WFUNC_READ_FW_VERSION             0x01
#define PROTO_WFUNC_READ_MACHTAG                0x02
#define PROTO_WFUNC_READ_STATUS_MASK            0x03
#define PROTO_WFUNC_READ_ERROR_MASK             0x04
#define PROTO_WFUNC_READ_CHANNEL_MASK           0x05
#define PROTO_WFUNC_READ_CHN_INDEX              0x06
#define PROTO_WFUNC_READ_CHN_MTU                0x07
#define PROTO_WFUNC_READ_CHN_MAX_WORDS          0x08
#define PROTO_WFUNC_READ_CHN_STATUS             0x09
#define PROTO_WFUNC_READ_CHN_MODE               0x0a
#define PROTO_WFUNC_READ_CHN_DEF_MTU            0x0b
#define PROTO_WFUNC_READ_CHN_ERROR_CODE         0x0c
#define PROTO_WFUNC_READ_USER                   0x0d

// write
#define PROTO_WFUNC_WRITE_CHN_INDEX             0x00
#define PROTO_WFUNC_WRITE_CHN_MTU               0x01
#define PROTO_WFUNC_WRITE_CHN_MAX_WORDS         0x02
#define PROTO_WFUNC_WRITE_USER                  0x03

// -- long function --
// read
#define PROTO_LFUNC_READ_CHN_OFFSET             0x00
#define PROTO_LFUNC_READ_USER                   0x01

// write
#define PROTO_LFUNC_WRITE_USER                  0x00

// -- status flag --
#define PROTO_STATUS_MASK_RX_PENDING            0x3f
#define PROTO_STATUS_MASK_ERROR                 0x40
#define PROTO_STATUS_MASK_BUSY                  0x80

#endif
