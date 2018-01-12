#ifndef TESTS_PALOMA_H
#define TESTS_PALOMA_H

#include "test.h"

int test_paloma_init_exit(test_t *t, test_param_t *p);
int test_handler_open_close(test_t *t, test_param_t *p);

#define TESTS_PALOMA_ALL \
  { test_paloma_init_exit, "pie", "paloma init/exit" }, \
  { test_handler_open_close, "hoc", "handler open/close" }, \

#endif
