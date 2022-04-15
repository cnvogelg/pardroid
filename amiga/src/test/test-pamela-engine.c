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

#include "pamela_engine.h"
#include "pamela.h"
#include "pamela-engine-testsuite.h"

#include "test.h"
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
  TESTS_PAMELA
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
  pam_eng_test_data_t pet;

  /* First parse args */
  args = ReadArgs(TEMPLATE, (LONG *)&params, NULL);
  if(args == NULL) {
    PrintFault(IoErr(), "Args Error");
    return RETURN_ERROR;
  }

  int res = RETURN_ERROR;

  /* setup pamela */
  int init_res;
  pet.engine = pamela_engine_init((struct Library *)SysBase, &init_res);
  if(pet.engine != NULL) {

    /* alloc port */
    pet.port = CreateMsgPort();
    if(pet.port != NULL) {
      pet.req = (struct IOPamReq *)CreateIORequest(pet.port, sizeof(*pet.req));
      if(pet.req != NULL) {

        /* setup test */
        test_param_t param;
        param.user_data = &pet;
        setup_test_config(&param);

        /* run test */
        res = test_main(all_tests, &param);

        DeleteIORequest(pet.req);
      } else {
        PutStr("no mem for req!\n");
      }
      DeleteMsgPort(pet.port);
    } else {
      PutStr("no mem for port\n");
    }
    pamela_engine_exit(pet.engine);
  } else {
    Printf("engine error %ld: %s -> ABORT!\n", init_res, pamela_perror(init_res));
  }

  /* Finally free args */
  FreeArgs(args);
  return res;
}