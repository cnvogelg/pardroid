#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include <string.h>
#include <stdio.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "parbox.h"
#include "proto.h"
#include "bench.h"

static const char *TEMPLATE =
  "L=Loop/S,"
  "N=Num/K/N,"
  "Bench/K";
typedef struct {
  ULONG loop;
  ULONG *num;
  char *bench;
} params_t;
static params_t params = { 0, NULL, "action" };

static ULONG get_num(void)
{
  if(params.num == NULL) {
    return 100;
  } else {
    return *params.num;
  }
}

/* benchmark functions */

static void bench_action(void *user_data)
{
  parbox_handle_t *pb = (parbox_handle_t *)user_data;
  proto_handle_t *ph = pb->proto;
  PutStr("action bench");

  ULONG deltas[2] = { 0,0 };
  int error = proto_action_bench(ph, PROTO_ACTION_PING, deltas);
  Printf("-> result=%ld, deltas=%lu, %lu\n",
      (LONG)error, deltas[0], deltas[1]);
}

/* benchmark table */

static bench_def_t all_benches[] = {
  { bench_action, "action", "test action command latency" },
  { NULL, NULL, NULL }
};

int dosmain(void)
{
  struct RDArgs *args;
  parbox_handle_t pb;

  /* First parse args */
  args = ReadArgs(TEMPLATE, (LONG *)&params, NULL);
  if(args == NULL) {
    PrintFault(IoErr(), "Args Error");
    return RETURN_ERROR;
  }

  int res = RETURN_ERROR;

  /* setup parbox */
  res = parbox_init(&pb, (struct Library *)SysBase);
  if(res == PARBOX_OK) {

    /* run test */
    res = bench_main(all_benches, params.bench, &pb);

    parbox_exit(&pb);
  } else {
    PutStr(parbox_perror(res));
    PutStr(" -> ABORT\n");
  }

  /* Finally free args */
  FreeArgs(args);
  return res;
}
