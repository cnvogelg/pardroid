#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "bench.h"
#include "bench_main.h"

static const char *TEMPLATE =
  "L=Loop/S,"
  "N=Num/K/N,"
  "Bench/K,"
  "Size/K/N,"
  "V=Verbose/S";
typedef struct {
  ULONG loop;
  ULONG *num;
  char *bench;
  ULONG *size;
  ULONG verbose;
} params_t;
static params_t params = { 0, NULL, NULL, NULL, 0 };

int dosmain(void)
{
  struct RDArgs *args;

  /* First parse args */
  args = ReadArgs(TEMPLATE, (LONG *)&params, NULL);
  if(args == NULL) {
    PrintFault(IoErr(), "Args Error");
    return RETURN_ERROR;
  }

  if(bench_get_verbose()) {
    Printf("Param: Size=%ld, Num=%ld\n", bench_get_size(), bench_get_num());
  }

  /* init */
  int res = RETURN_ERROR;
  void *user_data = bench_api_init();
  if(user_data != NULL) {
    /* run test */
    res = bench_main(all_benches, params.bench, user_data);

    /* exit */
    bench_api_exit(user_data);
  }

  /* Finally free args */
  FreeArgs(args);
  return res;
}

ULONG bench_get_num(void)
{
  if(params.num == NULL) {
    return 100;
  } else {
    return *params.num;
  }
}

ULONG bench_get_size(void)
{
  if(params.size == NULL) {
    return 1024;
  } else {
    ULONG s = *params.size;
    if(s&1) {
      s++;
    }
    return s;
  }
}

ULONG bench_get_verbose(void)
{
  return params.verbose;
}
