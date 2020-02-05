#ifndef PROTO_TEST_SHARED_H
#define PROTO_TEST_SHARED_H

// test proto: actions
#define PROTO_ACTION_TEST_SIGNAL                0x04
#define PROTO_ACTION_TEST_BUSY_LOOP             0x05

// proto test: read word
#define PROTO_WFUNC_READ_TEST_FW_ID             0x00
#define PROTO_WFUNC_READ_TEST_FW_VERSION        0x01
#define PROTO_WFUNC_READ_TEST_MACHTAG           0x02
#define PROTO_WFUNC_READ_TEST_FLAGS             0x03
#define PROTO_WFUNC_READ_TEST_VALUE             0x04
#define PROTO_WFUNC_READ_TEST_MAX_WORDS         0x05

// proto test: write word
#define PROTO_WFUNC_WRITE_TEST_FLAGS            0x00
#define PROTO_WFUNC_WRITE_TEST_VALUE            0x01

// proto test: read long
#define PROTO_LFUNC_READ_TEST_VALUE             0x00
#define PROTO_LFUNC_READ_TEST_OFFSET            0x01
// proto test: write long
#define PROTO_LFUNC_WRITE_TEST_VALUE            0x00

// -- test flags --
#define PROTO_TEST_FLAGS_USE_SPI                0x01
#define PROTO_TEST_FLAGS_MSG_ERROR              0x02
#define PROTO_TEST_FLAGS_CANCEL_TRANSFER        0x04

#endif
