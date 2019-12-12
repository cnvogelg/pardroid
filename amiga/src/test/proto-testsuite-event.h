#ifndef PROTO_TESTSUITE_EVENT_H
#define PROTO_TESTSUITE_EVENT_H

#include "test.h"

int test_timer_sig(test_t *t, test_param_t *p);
int test_event_sig(test_t *t, test_param_t *p);
int test_event_sig2(test_t *t, test_param_t *p);
int test_event_busy(test_t *t, test_param_t *p);

#define TESTS_PROTO_EVENT \
  { test_timer_sig, "tis", "timer signal"}, \
  { test_event_sig, "trs", "trigger signal"}, \
  { test_event_sig2, "trs2", "two trigger signals"}, \
  { test_event_busy, "tbs", "trigger after busy signal"}, \

#endif
