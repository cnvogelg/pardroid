#ifndef PROTO_TESTSUITE_H
#define PROTO_TESTSUITE_H

#include "test.h"

void tests_pamela_config(UWORD size, UWORD bias, UWORD add_size, UWORD sub_size,
                         UBYTE channel, UWORD mtu);

int test_channel_init_exit(test_t *t, test_param_t *p);
int test_channel_open_close(test_t *t, test_param_t *p);
int test_channel_write(test_t *t, test_param_t *p);
int test_channel_read(test_t *t, test_param_t *p);

#define TESTS_PAMELA_ALL \
  { test_channel_init_exit, "ie", "init/exit channel" }, \
  { test_channel_open_close, "oc", "open/close channel" }, \
  { test_channel_write, "w", "write message on channel" }, \
  { test_channel_read, "r", "read message from channel" }, \
 
#endif
