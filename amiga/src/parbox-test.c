#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include <string.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "parbox.h"
#include "proto.h"

static const char *TEMPLATE = "L=Loop/S,O=Once/S,Test/K,V=Verbose/S,C=Count/K/N";
typedef struct {
  ULONG loop;
  ULONG once;
  char *test;
  ULONG verbose;
  ULONG *count;
} params_t;
static params_t params;

/* test setup */
typedef int (*test_func_t)(parbox_handle_t *pb, ULONG i);
typedef struct {
  test_func_t   func;
  const char   *name;
  const char   *description;
} test_t;

static int test_ping(parbox_handle_t *pb, ULONG i)
{
  return proto_ping(pb->proto);
}

/* define tests */
static test_t all_tests[] = {
  { test_ping, "ping", "ping parbox device" },
  { NULL, NULL, NULL }
};

static int run_test(parbox_handle_t *pb, test_t *test)
{
  /* determine number of runs */
  ULONG num = 0;
  if(params.once) {
    num = 1;
  }
  else if(params.loop) {
    num = 0;
  }
  else if(params.count) {
    num = *params.count;
  }
  else {
    PutStr("Invalid test repeats given!\n");
    return RETURN_ERROR;
  }

  /* report test */
  Printf("Test: %s, Count: %ld  [%s]\n", test->name, num, test->description);

  return test->func(pb,0);
}

static test_t *pick_test(const char *name)
{
  /* default */
  if(name == NULL) {
    return &all_tests[0];
  }
  /* search test */
  test_t *t = all_tests;
  while(t->name != NULL) {
    if(strcmp(name, t->name) == 0) {
      return t;
    }
    t++;
  }
  return NULL;
}

int main(int argc, char **argv)
{
  struct RDArgs *args;
  parbox_handle_t pb;

  /* First parse args */
  args = ReadArgs(TEMPLATE, (LONG *)&params, NULL);
  if(args == NULL) {
    return RETURN_ERROR;
  }

  int res = RETURN_ERROR;

  /* pick test function */
  test_t *test = pick_test(params.test);
  if(test == NULL) {
    PutStr("No test found!\n");
  } else {
    /* setup parbox */
    res = parbox_init(&pb, (struct Library *)SysBase);
    if(res == PARBOX_OK) {

      /* run tests */
      res = run_test(&pb, test);

      parbox_exit(&pb);
    } else {
      PutStr(parbox_perror(res));
      PutStr(" -> ABORT\n");
    }
  }

  /* Finally free args */
  FreeArgs(args);
  return res;
}
