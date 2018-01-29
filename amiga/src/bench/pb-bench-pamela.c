#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include <string.h>
#include <stdio.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "pamela.h"
#include "bench.h"

static const char *TEMPLATE =
  "L=Loop/S,"
  "N=Num/K/N,"
  "Bench/K,"
  "Size/K/N";
typedef struct {
  ULONG loop;
  ULONG *num;
  char *bench;
  ULONG *size;
} params_t;
static params_t params = { 0, NULL, NULL, NULL };

static ULONG get_num(void)
{
  if(params.num == NULL) {
    return 100;
  } else {
    return *params.num;
  }
}

static ULONG get_size(void)
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

/* ----- events helper ----- */

#define WAIT_S      0
#define WAIT_US     100000UL

static int run_with_events(pamela_handle_t *pb, bench_func_t func)
{
  /* init events */
  int res = pamela_init_events(pb);
  if(res != PAMELA_OK) {
    Printf("ERROR: pamela_init_events: %ld %s\n",
           (LONG)res, proto_perror(res));
    return res;
  }

  /* call test func */
  func(pb);

  /* cleanup events */
  pamela_exit_events(pb);

  return res;
}

/* ----- test mode setup ----- */

#define REG_TEST_MODE   (PROTO_REGOFFSET_USER + 7)

#define TEST_MODE_NORMAL  0
#define TEST_MODE_ECHO    1

static int set_test_mode(pamela_handle_t *pb, UBYTE mode)
{
  proto_handle_t *proto = pamela_get_proto(pb);
  /* set test mode */
  int res = reg_set(proto, REG_TEST_MODE, mode);
  if(res != 0) {
    Printf("ERROR: set_test_mode(%lu): %ld %s\n",
           (ULONG)mode, (LONG)res, proto_perror(res));
  }
  return res;
}

static int run_in_test_mode(pamela_handle_t *pb, bench_func_t func, UBYTE mode)
{
  /* set echo test mode */
  int res = set_test_mode(pb, mode);
  if(res != 0) {
    return res;
  }

  res = run_with_events(pb, func);

  /* set normal mode */
  set_test_mode(pb, TEST_MODE_NORMAL);

  return res;
}

/* benchmark functions */

static void bench_action(void *user_data)
{
  pamela_handle_t *pb = (pamela_handle_t *)user_data;
  proto_handle_t *ph = pamela_get_proto(pb);
  PutStr("action bench");

  ULONG deltas[2] = { 0,0 };
  int error = proto_action_bench(ph, PROTO_ACTION_PING, deltas);
  Printf("-> result=%ld, deltas=%lu, %lu\n",
      (LONG)error, deltas[0], deltas[1]);
}

/* ----- run loop helper ----- */

typedef int (*loop_func_t)(pamela_handle_t *ph, ULONG iter, void *user_data);

static void run_loop(pamela_handle_t *pb, ULONG num, ULONG size,
                     loop_func_t func, void *user_data)
{
  timer_handle_t *timer = pamela_get_timer(pb);
  proto_handle_t *proto = pamela_get_proto(pb);

  ULONG sum_size = 0;
  ULONG i;
  time_stamp_t start, end;
  timer_eclock_get(timer, &start);

  for(i=0;i<num;i++) {
    int status = func(pb, i, user_data);
    if(status != PROTO_RET_OK) {
      Printf("ERROR: %lu = %s\n", status, proto_perror(status));
      break;
    }
    sum_size += size;
  }

  timer_eclock_get(timer, &end);
  time_stamp_t delta;
  timer_eclock_delta(&end, &start, &delta);

  ULONG us = timer_eclock_to_us(timer, &delta);
  ULONG kbps = (sum_size * 1000UL) / us;
  Printf("Result #%04lu: data=%lu us=%lu kbps=%lu\n", i, sum_size, us, kbps);
}

typedef struct {
  UBYTE *buffer;
  ULONG  num_words;
} msgio_data_t;

/* ----- bench functions ----- */

static int func_msg_write(pamela_handle_t *pb, ULONG iter, void *user_data)
{
  proto_handle_t *ph = pamela_get_proto(pb);
  msgio_data_t *data = (msgio_data_t *)user_data;
  return proto_msg_write_single(ph, 0, data->buffer, data->num_words);
}

static int func_msg_read(pamela_handle_t *pb, ULONG iter, void *user_data)
{
  proto_handle_t *ph = pamela_get_proto(pb);
  msgio_data_t *data = (msgio_data_t *)user_data;
  UWORD size = data->num_words;
  return proto_msg_read_single(ph, 0, data->buffer, &size);
}

static int func_msg_write_read(pamela_handle_t *pb, ULONG iter, void *user_data)
{
  proto_handle_t *ph = pamela_get_proto(pb);
  msgio_data_t *data = (msgio_data_t *)user_data;
  int status = proto_msg_write_single(ph, 0, data->buffer, data->num_words);
  if(status != PROTO_RET_OK) {
    return status;
  }
  UWORD size = data->num_words;
  return proto_msg_read_single(ph, 0, data->buffer, &size);
}

static int func_msg_write_read_sig(pamela_handle_t *pb, ULONG iter, void *user_data)
{
  proto_handle_t *ph = pamela_get_proto(pb);
  msgio_data_t *data = (msgio_data_t *)user_data;
  status_data_t *status = pamela_get_status(pb);

  /* write message */
  int res = proto_msg_write_single(ph, 0, data->buffer, data->num_words);
  if(res != PROTO_RET_OK) {
    return res;
  }

  /* wait for read pending signal */
  ULONG got = pamela_wait_event(pb, WAIT_S, WAIT_US, 0);

  /* check status */
  if(status->flags != STATUS_FLAGS_PENDING) {
    Printf("got=%08lx event=%08lx\n", got, pamela_get_event_sigmask(pb));
    Printf("#%ld: No read pending?? flags=%lu\n", iter, (ULONG)status->flags);
  }

  /* read message back */
  UWORD size = data->num_words;
  return proto_msg_read_single(ph, 0, data->buffer, &size);
}


/* ----- benchmarks ----- */

static void bench_msg_write(void *user_data)
{
  pamela_handle_t *pb = (pamela_handle_t *)user_data;
  proto_handle_t *ph = pamela_get_proto(pb);
  ULONG num = get_num();
  ULONG size = get_size();
  Printf("message write: num=%lu, size=%lu\n", num, size);

  UBYTE *buf = AllocVec(size, MEMF_PUBLIC);
  if(buf == NULL) {
    PutStr("No Memory!\n");
    return;
  }

  ULONG num_words = size >> 1;
  msgio_data_t data = { buf, num_words };
  run_loop(pb, num, size, func_msg_write, &data);

  FreeVec(buf);
}

static void bench_msg_read(void *user_data)
{
  pamela_handle_t *pb = (pamela_handle_t *)user_data;
  proto_handle_t *ph = pamela_get_proto(pb);
  ULONG num = get_num();
  ULONG size = get_size();
  Printf("message read: num=%lu, size=%lu\n", num, size);

  UBYTE *buf = AllocVec(size, MEMF_PUBLIC);
  if(buf == NULL) {
    PutStr("No Memory!\n");
    return;
  }

  ULONG num_words = size >> 1;
  msgio_data_t data = { buf, num_words };
  run_loop(pb, num, size, func_msg_read, &data);

  FreeVec(buf);
}

static void bench_msg_write_read(void *user_data)
{
  pamela_handle_t *pb = (pamela_handle_t *)user_data;
  proto_handle_t *ph = pamela_get_proto(pb);
  ULONG num = get_num();
  ULONG size = get_size();
  Printf("message write/read: num=%lu, size=%lu\n", num, size);

  UBYTE *buf = AllocVec(size, MEMF_PUBLIC);
  if(buf == NULL) {
    PutStr("No Memory!\n");
    return;
  }

  ULONG num_words = size >> 1;
  msgio_data_t data = { buf, num_words };
  run_loop(pb, num, size * 2, func_msg_write_read, &data);

  FreeVec(buf);
}

static void run_msg_write_read_sig(void *user_data)
{
  pamela_handle_t *pb = (pamela_handle_t *)user_data;
  proto_handle_t *ph = pamela_get_proto(pb);
  ULONG num = get_num();
  ULONG size = get_size();
  Printf("message write/read+sig: num=%lu, size=%lu\n", num, size);

  UBYTE *buf = AllocVec(size, MEMF_PUBLIC);
  if(buf == NULL) {
    PutStr("No Memory!\n");
    return;
  }

  ULONG num_words = size >> 1;
  msgio_data_t data = { buf, num_words };
  run_loop(pb, num, size * 2, func_msg_write_read_sig, &data);

  FreeVec(buf);
}

static void bench_msg_write_read_sig(void *user_data)
{
  pamela_handle_t *pb = (pamela_handle_t *)user_data;
  run_in_test_mode(pb, run_msg_write_read_sig, TEST_MODE_ECHO);
}

/* benchmark table */

static bench_def_t all_benches[] = {
  { bench_action, "action", "test action command latency" },
  { bench_msg_write, "mw", "message write benchmark" },
  { bench_msg_read, "mr", "message read benchmark" },
  { bench_msg_write_read, "mwr", "message write/read benchmark" },
  { bench_msg_write_read_sig, "mwrs", "message write/read+sig benchmark" },
  { NULL, NULL, NULL }
};

int dosmain(void)
{
  struct RDArgs *args;
  pamela_handle_t *pb;

  /* First parse args */
  args = ReadArgs(TEMPLATE, (LONG *)&params, NULL);
  if(args == NULL) {
    PrintFault(IoErr(), "Args Error");
    return RETURN_ERROR;
  }

  int res = RETURN_ERROR;

  /* setup pamela */
  int init_res;
  pb = pamela_init((struct Library *)SysBase, &init_res, PAMELA_INIT_NORMAL);
  if(init_res == PAMELA_OK) {

    /* run test */
    res = bench_main(all_benches, params.bench, pb);

    pamela_exit(pb);
  } else {
    PutStr(pamela_perror(res));
    PutStr(" -> ABORT\n");
  }

  /* Finally free args */
  FreeArgs(args);
  return res;
}
