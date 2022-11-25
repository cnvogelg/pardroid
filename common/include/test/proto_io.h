#ifndef PROTO_IO_TEST_SHARED_H
#define PROTO_IO_TEST_SHARED_H

#define TEST_CHANNEL    7
#define TEST_PORT       54321
#define TEST_SEEK       0xdeadbeefUL
#define TEST_SIZE       4096
#define TEST_BUF_SIZE   512
#define TEST_BYTE_OFFSET 3

#define TEST_ERROR      0xbabe

#define TEST_DEFAULT_MTU 0x1234
#define TEST_MTU         0x4321

#define TEST_STATUS_ACTIVE      0x01
#define TEST_STATUS_READ_BUSY   0x02
#define TEST_STATUS_WRITE_BUSY  0x04

#endif
