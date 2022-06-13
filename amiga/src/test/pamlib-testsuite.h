#ifndef PAMLIB_TESTSUITE_H
#define PAMLIB_TESTSUITE_H

#include "test.h"

TEST_FUNC(test_init_exit);
TEST_FUNC(test_open_close);
TEST_FUNC(test_write);
TEST_FUNC(test_read);

#define TESTS_PAMELA \
  { test_init_exit, "ie", "init/exit pamlib" }, \
  { test_open_close, "oc", "open/close channel" }, \
  { test_write, "w", "write message on channel" }, \
  { test_read, "r", "read message from channel" }, \

#endif
