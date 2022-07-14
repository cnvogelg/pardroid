#ifndef PALOMA_TESTSUITE_H
#define PALOMA_TESTSUITE_H

#include "test.h"

TEST_FUNC(test_init_exit);
TEST_FUNC(test_load_save_reset);

#define TESTS_PALOMA \
  { test_init_exit, "ie", "init/exit paloma" }, \
  { test_load_save_reset, "lsr", "load/save/reset all params" }, \

#endif
