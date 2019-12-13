// proto_shared.h
// shared defines for parbox protocol

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
#define PROTO_CMD_CHN_SET_RX_SIZE               0x90
#define PROTO_CMD_CHN_SET_TX_SIZE               0xa0
#define PROTO_CMD_CHN_SET_RX_OFFSET             0xb0
#define PROTO_CMD_CHN_SET_TX_OFFSET             0xc0
#define PROTO_CMD_CHN_REQUEST_RX                0xd0
#define PROTO_CMD_CHN_CANCEL_RX                 0xe0
#define PROTO_CMD_CHN_CANCEL_TX                 0xf0

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

// test proto: actions
#define PROTO_ACTION_TEST_SIGNAL                0x04
#define PROTO_ACTION_TEST_BUSY_LOOP             0x05

// -- word function -- 
// read
#define PROTO_WFUNC_READ_FW_ID                  0x00
#define PROTO_WFUNC_READ_FW_VERSION             0x01
#define PROTO_WFUNC_READ_MACHTAG                0x02
#define PROTO_WFUNC_READ_STATUS_MASK            0x03
#define PROTO_WFUNC_READ_ERROR_MASK             0x04
#define PROTO_WFUNC_READ_CHN_INDEX              0x05
#define PROTO_WFUNC_READ_CHN_MTU                0x06
#define PROTO_WFUNC_READ_CHN_ERROR_CODE         0x07
#define PROTO_WFUNC_READ_CHN_PROPERTIES         0x08
#define PROTO_WFUNC_READ_CHN_DEF_MTU            0x09
#define PROTO_WFUNC_READ_USER                   0x0a

// write
#define PROTO_WFUNC_WRITE_CHN_INDEX             0x00
#define PROTO_WFUNC_WRITE_CHN_MTU               0x01
#define PROTO_WFUNC_WRITE_USER                  0x02

// bootloader: read word
#define PROTO_WFUNC_READ_BOOT_FW_ID             0x00
#define PROTO_WFUNC_READ_BOOT_FW_VERSION        0x01
#define PROTO_WFUNC_READ_BOOT_MACHTAG           0x02
#define PROTO_WFUNC_READ_BOOT_ROM_FW_ID         0x03
#define PROTO_WFUNC_READ_BOOT_ROM_FW_VERSION    0x04
#define PROTO_WFUNC_READ_BOOT_ROM_MACHTAG       0x05
#define PROTO_WFUNC_READ_BOOT_ROM_CRC           0x06
#define PROTO_WFUNC_READ_BOOT_PAGE_WORDS        0x07

// proto test: read word
#define PROTO_WFUNC_READ_TEST_FW_ID             0x00
#define PROTO_WFUNC_READ_TEST_FW_VERSION        0x01
#define PROTO_WFUNC_READ_TEST_MACHTAG           0x02
#define PROTO_WFUNC_READ_TEST_FLAGS             0x03
#define PROTO_WFUNC_READ_TEST_VALUE             0x04
#define PROTO_WFUNC_READ_TEST_GET_TX_SIZE       0x05
#define PROTO_WFUNC_READ_TEST_MAX_WORDS         0x06

// proto test: write word
#define PROTO_WFUNC_WRITE_TEST_FLAGS            0x00
#define PROTO_WFUNC_WRITE_TEST_VALUE            0x01

// -- long function --
// read
#define PROTO_LFUNC_READ_CHN_RX_OFFSET          0x00
#define PROTO_LFUNC_READ_CHN_TX_OFFSET          0x01
#define PROTO_LFUNC_READ_USER                   0x02

// write
#define PROTO_LFUNC_WRITE_USER                  0x00

// bootloader: read long
#define PROTO_LFUNC_READ_BOOT_ROM_SIZE          0x00
// bootloader: write long
#define PROTO_LFUNC_WRITE_BOOT_PAGE_ADDR        0x00

// proto test: read long
#define PROTO_LFUNC_READ_TEST_VALUE             0x00
#define PROTO_LFUNC_READ_TEST_RX_OFFSET         0x01
#define PROTO_LFUNC_READ_TEST_TX_OFFSET         0x02
// proto test: write long
#define PROTO_LFUNC_WRITE_TEST_VALUE            0x00

// -- status flag --
#define PROTO_STATUS_MASK_RX_PENDING            0x3f
#define PROTO_STATUS_MASK_ERROR                 0x40
#define PROTO_STATUS_MASK_BUSY                  0x80

// -- test flags --
#define PROTO_TEST_FLAGS_USE_SPI                0x01
#define PROTO_TEST_FLAGS_MSG_ERROR              0x02
#define PROTO_TEST_FLAGS_RX_REQUEST             0x10
#define PROTO_TEST_FLAGS_RX_CANCEL              0x20
#define PROTO_TEST_FLAGS_TX_CANCEL              0x40
