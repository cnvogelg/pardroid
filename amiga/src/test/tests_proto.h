#ifndef TESTS_PROTO_H
#define TESTS_PROTO_H

#include "test.h"

void tests_proto_config(UWORD size, UWORD bias, UWORD add_size, UWORD sub_size, UBYTE channel);

int test_ping(test_t *t, test_param_t *p);
int test_reset(test_t *t, test_param_t *p);
int test_func_write(test_t *t, test_param_t *p);
int test_func_read(test_t *t, test_param_t *p);
int test_func_write_read(test_t *t, test_param_t *p);
int test_offset_write_read(test_t *t, test_param_t *p);
int test_msg_empty(test_t *t, test_param_t *p);
int test_msg_tiny(test_t *t, test_param_t *p);
int test_msg_size(test_t *t, test_param_t *p);
int test_msg_size_chunks(test_t *t, test_param_t *p);
int test_msg_write(test_t *t, test_param_t *p);
int test_msg_read(test_t *t, test_param_t *p);
int test_status_read_pending(test_t *t, test_param_t *p);
int test_status_ack_irq(test_t *t, test_param_t *p);
int test_status_error(test_t *t, test_param_t *p);
int test_status_attach_detach(test_t *t, test_param_t *p);

#define TESTS_PROTO_ALL \
  { test_ping, "ping", "ping parbox device" }, \
  { test_reset, "reset", "reset parbox device" }, \
  { test_func_read, "fr", "read test function word" }, \
  { test_func_write, "fw", "write test function word" }, \
  { test_func_write_read, "fwr", "write/read test function word" }, \
  { test_offset_write_read, "owr", "write/read long offset" }, \
  { test_msg_empty, "me", "write/read empty message"}, \
  { test_msg_tiny, "mt", "write/read tiny 4 byte message"}, \
  { test_msg_size, "ms", "write/read messages of given size"}, \
  { test_msg_size_chunks, "msc", "write/read messages of given size in two chunks"}, \
  { test_msg_write, "mw", "write message of given size"}, \
  { test_msg_read, "mr", "read message of given size"}, \
  { test_status_read_pending, "sp", "test status read pending flag"}, \
  { test_status_ack_irq, "sa", "test ack irq on first pending read request"}, \
  { test_status_error, "se", "test status error code" }, \
  { test_status_attach_detach, "sad", "test attach/detach" },

#endif
