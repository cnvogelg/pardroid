#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include <string.h>
#include <stdio.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"
#include "types.h"
#include "arch.h"

#include "proto_io.h"

#include "test.h"
#include "proto-io-testsuite.h"
#include "fwid.h"

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
  TESTS_PROTO_IO
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
    ph = proto_io_init(penv);
    if(ph != NULL) {
      /* check firmware */
      WORD fw_id = 0;
      proto_dev_get_fw_id(ph, &fw_id);
      if(fw_id == FWID_TEST_PROTO_IO) {

        /* setup test */
        test_param_t param;
        param.user_data = ph;
        setup_test_config(&param);

        /* run test */
        res = test_main(all_tests, &param);
      
      } else {
        Printf("wrong firmware: %04lx\n", fw_id);
      }

      proto_io_exit(ph);
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
