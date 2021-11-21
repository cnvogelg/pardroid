#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include <string.h>
#include <stdio.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "proto_dev.h"

#include "test.h"
#include "proto-dev-testsuite.h"

static const char *TEMPLATE = "L=Loop/S,N=Num/K/N,Test/K,Delay/K/N,Verbose/S";
typedef struct {
  ULONG loop;
  ULONG *num;
  char *test;
  ULONG *delay;
  ULONG verbose;
} params_t;
static params_t params;

/* define tests */
static test_t all_tests[] = {
  TESTS_PROTO_DEV
  { NULL, NULL, NULL }
};

void setup_test_config(test_param_t *p)
{
  /* base parameter */
  ULONG num = 1;
  if(params.num) {
    num = *params.num;
  }
  if(params.loop) {
    num = 0;
  }
  ULONG delay = 0;
  if(params.delay) {
    delay = *params.delay;
  }

  p->num_iter = num;
  p->delay = delay;
  p->test_name = params.test;
  p->verbose = params.verbose;
}

int dosmain(void)
{
  struct RDArgs *args;
  proto_env_handle_t *penv;
  proto_handle_t *ph;

  /* First parse args */
  args = ReadArgs(TEMPLATE, (LONG *)&params, NULL);
  if(args == NULL) {
    PrintFault(IoErr(), "Args Error");
    return RETURN_ERROR;
  }

  int res = RETURN_ERROR;

  /* setup pamela */
  int init_res;
  penv = proto_env_init((struct Library *)SysBase, &init_res);
  if(penv != NULL) {
    ph = proto_dev_init(penv);
    if(ph != NULL) {
      /* setup test */
      test_param_t param;
      param.user_data = ph;
      setup_test_config(&param);

      /* run test */
      res = test_main(all_tests, &param);

      proto_dev_exit(ph);
    } else {
      PutStr("proto failed!\n");
    }
    proto_env_exit(penv);
  } else {
    PutStr(proto_env_perror(init_res));
    PutStr(" -> ABORT\n");
  }

  /* Finally free args */
  FreeArgs(args);
  return res;
}
