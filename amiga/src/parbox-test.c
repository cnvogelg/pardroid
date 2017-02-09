#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include <string.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "parbox.h"
#include "proto.h"

static const char *TEMPLATE = "L=Loop/S,N=Num/K/N,Test/K,Delay/K/N";
typedef struct {
  ULONG loop;
  ULONG *num;
  char *test;
  ULONG *delay;
} params_t;
static params_t params;

/* test setup */
struct test;
typedef int (*test_func_t)(parbox_handle_t *pb, struct test *t);
typedef struct test {
  test_func_t   func;
  const char   *name;
  const char   *description;
  /* error */
  ULONG         iter;
  const char   *error;
  const char   *section;
} test_t;

/* ----- tests ----- */

static int test_ping(parbox_handle_t *pb, test_t *t)
{
  int res = proto_ping(pb->proto);
  if(res == 0) {
    return 0;
  } else {
    t->error = proto_perror(res);
    t->section = "ping";
    return res;
  }
}

static int test_rw(parbox_handle_t *pb, test_t *t)
{
  UBYTE b[2];
  b[0] = (UBYTE)(t->iter & 0xff);
  b[1] = (UBYTE)(0xff - b[0]);

  /* write */
  int res = proto_test_write(pb->proto, b);
  if(res != 0) {
    t->error = proto_perror(res);
    t->section = "write";
    return res;
  }

  /* read back */
  UBYTE r[2];
  res = proto_test_read(pb->proto, r);
  if(res != 0) {
    t->error = proto_perror(res);
    t->section = "read";
    return res;
  }

  /* check */
  if(b[0] != r[0]) {
    t->error = "byte 0 mismatch";
    t->section = "compare";
    return 1;
  }
  if(b[1] != r[1]) {
    t->error = "byte 1 mismatch";
    t->section = "compare";
    return 1;
  }

  return 0;
}

/* define tests */
static test_t all_tests[] = {
  { test_ping, "ping", "ping parbox device" },
  { test_rw, "rw", "read/write 2 test bytes" },
  { NULL, NULL, NULL }
};

static int run_test(parbox_handle_t *pb, test_t *test)
{
  /* determine number of runs */
  ULONG num = 1;
  if(params.loop) {
    num = 0;
  }
  else if(params.num) {
    num = *params.num;
  }

  /* delay */
  ULONG delay = 2;
  if(params.delay) {
    delay = *params.delay;
  }

  /* report test */
  Printf("Test: %s, Count: %ld, Delay: %ld  [%s]\n",
    test->name, num, delay, test->description);

  int res;
  if(num == 1) {
    /* single run */
    test->iter = 0;
    res = test->func(pb, test);
    if(res == 0) {
      PutStr("Ok.\n");
    }
    else {
      Printf("Result: %ld  %s  in  '%s'\n", (LONG)res, test->error, test->section);
    }
  }
  else {
    /* loop */
    ULONG cnt = 0;
    int force = 0;
    ULONG num_failed = 0;
    while(1) {
      /* break? */
      if(SetSignal(0L,SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C) {
        PutStr("Abort.\n");
        break;
      }

      /* test func */
      test->iter = cnt++;
      res = test->func(pb, test);
      if(res == 0) {
        if((cnt & 0xff == 0) || force) {
          Printf("%06ld: ok\r", cnt);
          force = 0;
        }
      }
      else {
        Printf("%06ld: %ld  %s  in  '%s'\n", cnt, (LONG)res, test->error, test->section);
        force = 1;
        num_failed++;
      }

      /* done? */
      if(num == cnt) {
        break;
      }

      /* delay */
      if(delay > 0) {
        Delay(delay);
      }
    }
    Printf("Total: %ld, Failed: %ld\n", cnt, num_failed);
  }

  return res == 0 ? RETURN_OK : RETURN_WARN;
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

static void show_tests(void)
{
  test_t *t = all_tests;
  while(t->name != NULL) {
    Printf("%-16s  %s\n", t->name, t->description);
    t++;
  }
}

int dosmain(void)
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
    PutStr("Test not found! Available tests are:\n");
    show_tests();
  } else {
    /* setup parbox */
    res = parbox_init(&pb, (struct Library *)SysBase);
    if(res == PARBOX_OK) {

      /* run test */
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
