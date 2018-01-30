#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include <string.h>
#include <stdio.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "pamela.h"
#include "proto.h"
#include "test.h"
#include "tests_pamela.h"


static const char *TEMPLATE = "L=Loop/S,N=Num/K/N,Test/K,Delay/K/N,"
   "Bias/K/N,Size/K/N,"
   "AddSize/K/N,SubSize/K/N,"
   "Channel/K/N,Verbose/S";
typedef struct {
  ULONG loop;
  ULONG *num;
  char *test;
  ULONG *delay;
  ULONG *bias;
  ULONG *size;
  ULONG *add_size;
  ULONG *sub_size;
  ULONG *channel;
  ULONG verbose;
} params_t;
static params_t params;

/* define tests */
static test_t all_tests[] = {
  TESTS_PAMELA_ALL
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

  /* proto test setup */
  UWORD size = 0;
  if(params.size) {
    size = (UWORD)*params.size;
  }
  UWORD bias = 0;
  if(params.bias) {
    bias = (UWORD)*params.bias;
  }
  UWORD add_size = 0;
  if(params.add_size) {
    add_size = (UWORD)*params.add_size;
  }
  UWORD sub_size = 0;
  if(params.sub_size) {
    sub_size = (UWORD)*params.sub_size;
  }
  UBYTE channel = 0;
  if(params.channel) {
    channel = (UBYTE)*params.channel;
  }

  tests_proto_config(size, bias, add_size, sub_size, channel);
}

int dosmain(void)
{
  struct RDArgs *args;
  pamela_handle_t *ph;

  /* First parse args */
  args = ReadArgs(TEMPLATE, (LONG *)&params, NULL);
  if(args == NULL) {
    PrintFault(IoErr(), "Args Error");
    return RETURN_ERROR;
  }

  int res = RETURN_ERROR;

  /* setup pamela */
  int init_res;
  ph = pamela_init((struct Library *)SysBase, &init_res, PAMELA_INIT_NORMAL);
  if(init_res == PAMELA_OK) {

    /* setup test */
    test_param_t param;
    param.user_data = ph;
    setup_test_config(&param);

    /* run test */
    res = test_main(all_tests, &param);

    pamela_exit(ph);
  } else {
    PutStr(pamela_perror(init_res));
    PutStr(" -> ABORT\n");
  }

  /* Finally free args */
  FreeArgs(args);
  return res;
}
