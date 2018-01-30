#ifndef TEST_H
#define TEST_H

/* global test parameter */
typedef struct test_param {
  /* config */
  void         *user_data;
  ULONG         num_iter;
  ULONG         delay;
  const char   *test_name;
  ULONG         verbose;
  /* state */
  ULONG         iter;
  const char   *error;
  const char   *section;
  char          extra[40];
} test_param_t;

/* test setup */
struct test;
typedef int (*test_func_t)(struct test *t, test_param_t *param);
typedef struct test {
  test_func_t   func;
  const char   *name;
  const char   *description;
} test_t;

extern int test_main(test_t all_tests[], test_param_t *param);

#endif
