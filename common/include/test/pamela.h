#ifndef TEST_PAMELA_H
#define TEST_PAMELA_H

#define TEST_MAX_BUF_SIZE    512 * 3
#define TEST_BUF_SIZE        512
#define TEST_DEFAULT_MTU     512

#define TEST_ERROR_REQ_SIZE   113
#define TEST_ERROR_POLL_SIZE  114
#define TEST_ERROR_DONE_SIZE  115
#define TEST_SHORT_SIZE       116
#define TEST_REDUCED_SIZE     72

#define TEST_ERROR_READ      0x88
#define TEST_ERROR_WRITE     0x99

#define TEST_NUM_SLOTS       4
#define TEST_ERROR_SLOT      3

#define TEST_PORT_MIN        1000
#define TEST_PORT_MAX        1999

#define TEST_INVALID_PORT    2000
#define TEST_OPEN_ERROR_PORT 1999

#endif
