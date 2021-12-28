#ifndef PROTO_ATOM_TESTSUITE_DEV_H
#define PROTO_ATOM_TESTSUITE_DEV_H

#include "test.h"

TEST_FUNC(test_event_mask);
TEST_FUNC(test_mtu);
TEST_FUNC(test_max_channels);
TEST_FUNC(test_open_close);
TEST_FUNC(test_reset);
TEST_FUNC(test_seek_tell);
TEST_FUNC(test_read_req_res);
TEST_FUNC(test_write_req_res);
TEST_FUNC(test_rw_data);

#define TESTS_PROTO_IO \
  { test_event_mask, "em", "event mask"}, \
  { test_mtu, "mtu", "test mtu config"}, \
  { test_max_channels, "mc", "max channels"}, \
  { test_open_close, "oc", "open/close"}, \
  { test_reset, "rst", "reset"}, \
  { test_seek_tell, "st", "seek/tell"}, \
  { test_read_req_res, "rrr", "read request/result"}, \
  { test_write_req_res, "wrr", "write request/result"}, \
  { test_rw_data, "rwd", "read/write data"}, \

#endif
