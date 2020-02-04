#ifndef PROTO_TESTSUITE_MSG_H
#define PROTO_TESTSUITE_MSG_H

#include "test.h"

int test_msg_empty(test_t *t, test_param_t *p);
int test_msg_tiny(test_t *t, test_param_t *p);
int test_msg_size(test_t *t, test_param_t *p);
int test_msg_size_max(test_t *t, test_param_t *p);
int test_msg_size_chunks(test_t *t, test_param_t *p);
int test_msg_size_chunks_len(test_t *t, test_param_t *p);
int test_msg_write(test_t *t, test_param_t *p);
int test_msg_read(test_t *t, test_param_t *p);
int test_msg_write_busy(test_t *t, test_param_t *p);
int test_msg_read_busy(test_t *t, test_param_t *p);
int test_msg_write_spi(test_t *t, test_param_t *p);
int test_msg_read_spi(test_t *t, test_param_t *p);

#define TESTS_PROTO_MSG \
  { test_msg_empty, "me", "write/read empty message"}, \
  { test_msg_tiny, "mt", "write/read tiny 4 byte message"}, \
  { test_msg_size, "ms", "write/read messages of given size"}, \
  { test_msg_size_max, "msx", "write/read messages of max size"}, \
  { test_msg_size_chunks, "msc", "write/read messages of given size in two chunks"}, \
  { test_msg_size_chunks_len, "msc", "write/read messages of given size in two chunks with len limit"}, \
  { test_msg_write, "mw", "write message of given size"}, \
  { test_msg_read, "mr", "read message of given size"}, \
  { test_msg_write_busy, "mwb", "write message while being busy"}, \
  { test_msg_read_busy, "mrb", "read message while being busy"}, \
  { test_msg_write_spi, "mws", "write message to SPI"}, \
  { test_msg_read_spi, "mrs", "read message from SPI"}, \

#endif
