#ifndef PROTO_TESTSUITE_ENV_H
#define PROTO_TESTSUITE_ENV_H

#include "test.h"

int test_timer_sig(test_t *t, test_param_t *p);
int test_event_sig(test_t *t, test_param_t *p);
int test_event_sig2(test_t *t, test_param_t *p);

#define TESTS_PROTO_ENV \
  { test_timer_sig, "tis", "timer signal"}, \
  { test_event_sig, "trs", "trigger signal"}, \
  { test_event_sig2, "trs2", "two trigger signals"}, \

#endif
