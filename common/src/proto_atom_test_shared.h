#ifndef PROTO_ATOM_TEST_SHARED_H
#define PROTO_ATOM_TEST_SHARED_H

// commands
#define TEST_ACTION           1
#define TEST_READ_WORD        2
#define TEST_WRITE_WORD       3
#define TEST_READ_LONG        4
#define TEST_WRITE_LONG       5
#define TEST_READ_BLOCK       6
#define TEST_WRITE_BLOCK      7
#define TEST_READ_BLOCK_SPI   8
#define TEST_WRITE_BLOCK_SPI  9
#define TEST_PULSE_IRQ        10
#define TEST_SET_BUSY         11
#define TEST_CLR_BUSY         12

// constants
#define TEST_BUF_SIZE         512
#define TEST_WORD             0xbabe
#define TEST_LONG             0xcafebabeUL
#define TEST_BYTE_OFFSET      0xab

#endif
