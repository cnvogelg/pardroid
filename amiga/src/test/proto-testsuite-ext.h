#ifndef PROTO_TESTSUITE_EXT_H
#define PROTO_TESTSUITE_EXT_H

#include "test.h"

int test_chn_set_get_size(test_t *t, test_param_t *p);
int test_chn_set_get_offset(test_t *t, test_param_t *p);
int test_chn_cancel_transfer(test_t *t, test_param_t *p);

#define TESTS_PROTO_EXT \
  { test_chn_set_get_size, "sgs", "set/get size"}, \
  { test_chn_set_get_offset, "sgo", "set/get offset"}, \
  { test_chn_cancel_transfer, "ct", "cancel transfer"}, \

#endif
