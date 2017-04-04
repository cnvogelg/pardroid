#ifndef TESTS_PROTO_H
#define TESTS_PROTO_H

#include "test.h"

int test_ping(test_t *t, test_param_t *p);
int test_reset(test_t *t, test_param_t *p);
int test_reg_write(test_t *t, test_param_t *p);
int test_reg_read(test_t *t, test_param_t *p);
int test_reg_write_read(test_t *t, test_param_t *p);
int test_msg_empty(test_t *t, test_param_t *p);
int test_msg_tiny(test_t *t, test_param_t *p);

#define TESTS_PROTO_ALL \
  { test_ping, "ping", "ping parbox device" },
  { test_reset, "reset", "reset parbox device" },
  { test_reg_read, "pr", "read test register word" },
  { test_reg_write, "pw", "write test register word" },
  { test_reg_write_read, "pwr", "write/read test register word" },
  { test_msg_empty, "me", "write/read empty message"},
  { test_msg_tiny, "mt", "write/read tiny 4 byte message"},

#endif
