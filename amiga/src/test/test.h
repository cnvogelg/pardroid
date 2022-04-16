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
  ULONG         port;
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

#define TEST_FUNC(bla) \
  int bla(test_t *t, test_param_t *p)

#define CHECK_RES(res, sec) \
  if (res != 0) \
  { \
    p->error = proto_atom_perror(res); \
    p->section = sec; \
    return res; \
  }

#define CHECK_EQUAL(got, exp, sec) \
  if (got != exp) \
  { \
    p->error = "value mismatch"; \
    p->section = sec; \
    sprintf(p->extra, "got=%04x exp=%04x", got, exp); \
    return 1; \
  }

#define CHECK_LEQUAL(got, exp, sec) \
  if (got != exp) \
  { \
    p->error = "value mismatch"; \
    p->section = sec; \
    sprintf(p->extra, "got=%04lx exp=%04lx", got, exp); \
    return 1; \
  }

#define CHECK_NOT_NULL(ptr, sec) \
  if(ptr == NULL) { \
    p->error = "pointer is NULL"; \
    p->section = sec; \
    return 1; \
  }

#endif
