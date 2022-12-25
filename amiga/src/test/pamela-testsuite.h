#ifndef PAMELA_TESTSUITE_H
#define PAMELA_TESTSUITE_H

#include "test.h"

TEST_FUNC(test_init_exit);
TEST_FUNC(test_open_close);
TEST_FUNC(test_open_port_error);
TEST_FUNC(test_open_own_error);
TEST_FUNC(test_reset);
TEST_FUNC(test_write);
TEST_FUNC(test_read);
TEST_FUNC(test_write_odd);
TEST_FUNC(test_read_odd);
TEST_FUNC(test_write_short);
TEST_FUNC(test_read_short);
TEST_FUNC(test_write_error_req);
TEST_FUNC(test_read_error_req);
TEST_FUNC(test_write_error_poll);
TEST_FUNC(test_read_error_poll);
TEST_FUNC(test_write_error_done);
TEST_FUNC(test_read_error_done);
TEST_FUNC(test_seek_tell);
TEST_FUNC(test_get_set_mtu);

#define TESTS_PAMELA \
  { test_init_exit, "ie", "init/exit pamela" }, \
  { test_open_close, "oc", "open/close channel" }, \
  { test_open_port_error, "ope", "open invalid port" }, \
  { test_open_own_error, "ooe", "open own error code" }, \
  { test_reset, "rst", "reset channel" }, \
  { test_write, "w", "write message on channel" }, \
  { test_read, "r", "read message from channel" }, \
  { test_write_odd, "wo", "write odd sized message on channel" }, \
  { test_read_odd, "ro", "read odd sized message from channel" }, \
  { test_write_short, "ws", "write short sized message on channel" }, \
  { test_read_short, "rs", "read short sized message from channel" }, \
  { test_write_error_req, "wer", "write req error on channel" }, \
  { test_read_error_req, "rer", "read req error on channel" }, \
  { test_write_error_poll, "wep", "write poll error on channel" }, \
  { test_read_error_poll, "rep", "read poll error on channel" }, \
  { test_write_error_done, "wed", "write done error on channel" }, \
  { test_read_error_done, "red", "read done error on channel" }, \
  { test_seek_tell, "st", "seek/tell" }, \
  { test_get_set_mtu, "gsm", "get/set mtu" }, \
 
#endif
