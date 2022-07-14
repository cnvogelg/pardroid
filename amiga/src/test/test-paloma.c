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

#include "paloma_lib.h"
#include "paloma-testsuite.h"
#include "parbox/ports.h"

#include "test.h"
#include "fwid.h"

static const char *TEMPLATE = "L=Loop/S,N=Num/K/N,Test/K,Delay/K/N,Verbose/S,Port/K/N";
typedef struct {
  ULONG loop;
  ULONG *num;
  char *test;
  ULONG *delay;
  ULONG verbose;
  ULONG *port;
} params_t;
static params_t params;

/* define tests */
static test_t all_tests[] = {
  TESTS_PALOMA
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
  ULONG port = 1000;
  if(params.port) {
    port = *params.port;
  }

  p->num_iter = num;
  p->delay = delay;
  p->test_name = params.test;
  p->verbose = params.verbose;
  p->port = port;
}

int dosmain(void)
{
  struct RDArgs *args;
  char *device = "pamela.device";
  pamela_devinfo_t devinfo;
  pamlib_handle_t *pam;
  paloma_handle_t *pal;

  /* First parse args */
  args = ReadArgs(TEMPLATE, (LONG *)&params, NULL);
  if(args == NULL) {
    PrintFault(IoErr(), "Args Error");
    return RETURN_ERROR;
  }

  int res = RETURN_ERROR;

  /* setup pamela */
  int init_res;
  pam = pamlib_init((struct Library *)SysBase, &init_res, device);
  if(pam != NULL) {

    init_res = pamlib_devinfo(pam, &devinfo);
    if(init_res == PAMELA_OK) {
      UWORD fw_id = devinfo.firmware_id;
      if(fw_id == FWID_TEST_PALOMA) {

        /* init paloma */
        pal = paloma_init((struct Library *)SysBase, pam, PALOMA_DEFAULT_PORT, &init_res);
        if(pal != NULL) {

          /* setup test */
          test_param_t param;
          param.user_data = pal;
          setup_test_config(&param);

          /* run test */
          res = test_main(all_tests, &param);

        } else {
          Printf("paloma failed: %ld\n", init_res);
        }

      } else {
        Printf("wrong firmware: %04x\n", fw_id);
      }
    } else {
      Printf("devinfo failed: %ld\n", init_res, (LONG)pamela_perror(init_res));
    }
    
    pamlib_exit(pam);
  } else {
    Printf("init ABORT: %ld %s!\n", init_res, (LONG)pamela_perror(init_res));
  }

  /* Finally free args */
  FreeArgs(args);
  return res;
}
