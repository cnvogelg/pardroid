#ifndef PAMLIB_TESTSUITE_H
#define PAMLIB_TESTSUITE_H

#include "test.h"

TEST_FUNC(test_init_exit);
TEST_FUNC(test_open_close);
TEST_FUNC(test_write);
TEST_FUNC(test_read);
TEST_FUNC(test_write_odd);
TEST_FUNC(test_read_odd);
TEST_FUNC(test_seek_tell);
TEST_FUNC(test_get_set_mtu);

#define TESTS_PAMELA \
  { test_init_exit, "ie", "init/exit pamlib" }, \
  { test_open_close, "oc", "open/close channel" }, \
  { test_write, "w", "write message on channel" }, \
  { test_read, "r", "read message from channel" }, \
  { test_write_odd, "wo", "write odd sized message on channel" }, \
  { test_read_odd, "ro", "read odd sized message from channel" }, \
  { test_seek_tell, "st", "seek/tell" }, \
  { test_get_set_mtu, "gsm", "get/set mtu" }, \

#endif
