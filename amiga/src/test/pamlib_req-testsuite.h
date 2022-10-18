#ifndef PAMLIB_REQ_TESTSUITE_H
#define PAMLIB_REQ_TESTSUITE_H

#include "test.h"

TEST_FUNC(test_init_exit);
TEST_FUNC(test_open_close);
TEST_FUNC(test_transfer);
TEST_FUNC(test_transfer_odd);

#define TESTS_PAMELA_REQ \
  { test_init_exit, "ie", "init/exit pamlib" }, \
  { test_open_close, "oc", "open/close channel for req" }, \
  { test_transfer, "t", "handle a single req/rep transfer" }, \
  { test_transfer_odd, "to", "handle a single req/rep transfer with odd size" }, \

#endif
