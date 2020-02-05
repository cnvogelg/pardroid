#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include <string.h>
#include <stdio.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "proto.h"
#include "proto_env.h"

#include "test.h"
#include "pamela-testsuite.h"

static const char *TEMPLATE = "L=Loop/S,N=Num/K/N,Test/K,Delay/K/N,"
   "Bias/K/N,Size/K/N,"
   "AddSize/K/N,SubSize/K/N,"
   "Channel/K/N,Verbose/S,"
   "MTU/K/N";
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
  ULONG *mtu;
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
  UWORD size = 512;
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
  WORD mtu = 64;
  if(params.mtu) {
    mtu = (UWORD)*params.mtu;
  }

  tests_pamela_config(size, bias, add_size, sub_size, channel, mtu);
}

int dosmain(void)
{
  struct RDArgs *args;
  proto_env_handle_t *ph;

  /* First parse args */
  args = ReadArgs(TEMPLATE, (LONG *)&params, NULL);
  if(args == NULL) {
    PrintFault(IoErr(), "Args Error");
    return RETURN_ERROR;
  }

  int res = RETURN_ERROR;

  /* setup pamela */
  int init_res;
  ph = proto_env_init((struct Library *)SysBase, &init_res);
  if(ph != NULL) {

    /* reset box */
    proto_handle_t *prh = proto_env_get_proto(ph);
    int pres = proto_reset(prh);
    if(pres != PROTO_RET_OK) {
      PutStr(proto_perror(pres));
      PutStr(" -> RESET box failed!\n");
    } else {
      /* setup test */
      test_param_t param;
      param.user_data = ph;
      setup_test_config(&param);

      /* run test */
      res = test_main(all_tests, &param);
    }

    /* return to knok */
    pres = proto_knok(prh);
    if(pres != PROTO_RET_OK) {
      PutStr(proto_perror(pres));
      PutStr(" -> entering KNOK failed!\n");
    }

    proto_env_exit(ph);
  } else {
    PutStr(proto_env_perror(init_res));
    PutStr(" -> ABORT\n");
  }

  /* Finally free args */
  FreeArgs(args);
  return res;
}
