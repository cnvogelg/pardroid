#ifndef PROTO_ATOM_TESTSUITE_DEV_H
#define PROTO_ATOM_TESTSUITE_DEV_H

#include "test.h"

int test_reset(test_t *t, test_param_t *p);

#define TESTS_PROTO_DEV \
  { test_reset, "r", "reset"}, \

#endif
