#ifndef PROTO_TESTSUITE_H
#define PROTO_TESTSUITE_H

#include "test.h"

void tests_proto_config(UWORD size, UWORD bias, UWORD add_size, UWORD sub_size,
                        UBYTE channel);

int test_reset(test_t *t, test_param_t *p);
int test_knok(test_t *t, test_param_t *p);
int test_ping(test_t *t, test_param_t *p);
int test_ping_busy(test_t *t, test_param_t *p);

int test_wfunc_write_read(test_t *t, test_param_t *p);
int test_lfunc_write_read(test_t *t, test_param_t *p);
int test_wfunc_busy(test_t *t, test_param_t *p);
int test_lfunc_busy(test_t *t, test_param_t *p);

int test_offset_write_read(test_t *t, test_param_t *p);
int test_offset_busy(test_t *t, test_param_t *p);

int recover_from_busy(proto_handle_t *proto, test_param_t *p);

// test parameter
extern UWORD test_size;
extern UWORD test_bias;
extern UWORD test_add_size;
extern UWORD test_sub_size;
extern UBYTE test_channel;

#define TESTS_PROTO_ALL \
  { test_reset, "reset", "reset parbox device" }, \
  { test_knok, "knok", "enter/leave knok mode of device" }, \
  { test_ping, "ping", "ping parbox device" }, \
  { test_ping_busy, "pingb", "ping while busy" }, \
  { test_wfunc_write_read, "wf", "write/read test function word" }, \
  { test_lfunc_write_read, "lf", "write/read test function long" }, \
  { test_wfunc_busy, "wfb", "write/read function word while busy" }, \
  { test_lfunc_busy, "lfb", "write/read function long while busy" }, \

#endif
