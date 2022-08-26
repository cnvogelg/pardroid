#ifndef PAMELA_TESTSUITE_H
#define PAMELA_TESTSUITE_H

#include "test.h"

TEST_FUNC(test_init_exit);
TEST_FUNC(test_open_close);
TEST_FUNC(test_write);
TEST_FUNC(test_read);
TEST_FUNC(test_seek_tell);
TEST_FUNC(test_get_set_mtu);

#define TESTS_PAMELA \
  { test_init_exit, "ie", "init/exit pamela" }, \
  { test_open_close, "oc", "open/close channel" }, \
  { test_write, "w", "write message on channel" }, \
  { test_read, "r", "read message from channel" }, \
  { test_seek_tell, "st", "seek/tell" }, \
  { test_get_set_mtu, "gsm", "get/set mtu" }, \
 
#endif
