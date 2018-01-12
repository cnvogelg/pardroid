#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include <string.h>
#include <stdio.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "paloma.h"
#include "proto.h"
#include "test.h"
#include "tests_paloma.h"


static const char *TEMPLATE = "L=Loop/S,N=Num/K/N,Test/K,Delay/K/N";
typedef struct {
  ULONG loop;
  ULONG *num;
  char *test;
  ULONG *delay;
} params_t;
static params_t params;

/* define tests */
static test_t all_tests[] = {
  TESTS_PALOMA_ALL
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
}

int dosmain(void)
{
  struct RDArgs *args;
  pamela_handle_t pam;
  paloma_handle_t pal;

  /* First parse args */
  args = ReadArgs(TEMPLATE, (LONG *)&params, NULL);
  if(args == NULL) {
    PrintFault(IoErr(), "Args Error");
    return RETURN_ERROR;
  }

  int res = RETURN_ERROR;

  /* setup pamela */
  res = pamela_init(&pam, (struct Library *)SysBase);
  if(res == PAMELA_OK) {

    /* setup paloma */
    res = paloma_init(&pal, &pam);
    if(res == PALOMA_OK) {

      /* setup test */
      test_param_t param;
      param.user_data = &pal;
      setup_test_config(&param);

      /* run test */
      res = test_main(all_tests, &param);

      paloma_exit(&pal);
    } else {
      PutStr(paloma_perror(res));
      PutStr(" -> ABORT\n");
    }
    pamela_exit(&pam);
  } else {
    PutStr(pamela_perror(res));
    PutStr(" -> ABORT\n");
  }

  /* Finally free args */
  FreeArgs(args);
  return res;
}
