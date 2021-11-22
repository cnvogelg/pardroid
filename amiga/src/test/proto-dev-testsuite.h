#ifndef PROTO_ATOM_TESTSUITE_DEV_H
#define PROTO_ATOM_TESTSUITE_DEV_H

#include "test.h"

int test_ping(test_t *t, test_param_t *p);
int test_fwid(test_t *t, test_param_t *p);
int test_fwver(test_t *t, test_param_t *p);
int test_machtag(test_t *t, test_param_t *p);
int test_drvtok(test_t *t, test_param_t *p);

#define TESTS_PROTO_DEV \
  { test_ping, "p", "ping device"}, \
  { test_fwid, "i", "get fw id"}, \
  { test_fwver, "v", "get fw version"}, \
  { test_machtag, "m", "get machtag"}, \
  { test_drvtok, "t", "set/get driver token"}, \

#endif
