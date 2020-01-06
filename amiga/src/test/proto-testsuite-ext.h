#ifndef PROTO_TESTSUITE_EXT_H
#define PROTO_TESTSUITE_EXT_H

#include "test.h"

int test_chn_set_get_rx_size(test_t *t, test_param_t *p);
int test_chn_set_get_tx_size(test_t *t, test_param_t *p);
int test_chn_set_get_rx_offset(test_t *t, test_param_t *p);
int test_chn_set_get_tx_offset(test_t *t, test_param_t *p);
int test_chn_request_rx(test_t *t, test_param_t *p);
int test_chn_cancel_rx(test_t *t, test_param_t *p);
int test_chn_cancel_tx(test_t *t, test_param_t *p);

#define TESTS_PROTO_EXT \
  { test_chn_set_get_rx_size, "rxs", "set/get rx size"}, \
  { test_chn_set_get_tx_size, "txs", "set/get tx size"}, \
  { test_chn_set_get_rx_offset, "rxo", "set/get rx offset"}, \
  { test_chn_set_get_tx_offset, "txo", "set/get tx offset"}, \
  { test_chn_request_rx, "rrx", "request rx"}, \
  { test_chn_cancel_rx, "crx", "cancel rx"}, \
  { test_chn_cancel_tx, "ctx", "cancel tx"}, \

#endif
