#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include <string.h>
#include <stdio.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "proto_env.h"
#include "bench.h"
#include "bench_main.h"

static void proto_err(int res)
{
  Printf("\nPROTO ERROR: %ld  %s\n", res, (LONG)proto_perror(res));
}

/* ----- events helper ----- */

#define WAIT_S      0
#define WAIT_US     100000UL

static ULONG run_with_events(bench_def_t *def, proto_env_handle_t *pb, bench_func_t func)
{
  /* init events */
  int res = proto_env_init_events(pb);
  if(res != PROTO_ENV_OK) {
    Printf("ERROR: proto_env_init_events: %ld %s!!\n",
           (LONG)res, (LONG)proto_perror(res));
    return 0;
  }

  /* call test func */
  ULONG val = func(def, pb);

  /* cleanup events */
  proto_env_exit_events(pb);

  return val;
}

/* ----- test mode setup ----- */

#define REG_TEST_MODE   (PROTO_REGOFFSET_USER + 7)

#define TEST_MODE_NORMAL  0
#define TEST_MODE_ECHO    1

static int set_test_mode(proto_env_handle_t *pb, UBYTE mode)
{
#if 0
  proto_handle_t *proto = proto_env_get_proto(pb);
  /* set test mode */
  int res = reg_set(proto, REG_TEST_MODE, mode);
  if(res != 0) {
    Printf("ERROR: set_test_mode(%lu): %ld %s\n",
           (ULONG)mode, (LONG)res, proto_perror(res));
  }
  return res;
#else
  return 0;
#endif
}

static ULONG run_in_test_mode(bench_def_t *def, proto_env_handle_t *pb, bench_func_t func, UBYTE mode)
{
  /* set echo test mode */
  int res = set_test_mode(pb, mode);
  if(res != 0) {
    return 0;
  }

  ULONG val = run_with_events(def, pb, func);

  /* set normal mode */
  set_test_mode(pb, TEST_MODE_NORMAL);

  return val;
}

/* ----- run loop helper ----- */

typedef int (*loop_func_t)(proto_env_handle_t *ph, ULONG iter,
                           UBYTE *buffer, UWORD num_words);

static ULONG run_kbps_loop(proto_env_handle_t *pb, loop_func_t func, ULONG size_mult)
{
  timer_handle_t *timer = proto_env_get_timer(pb);
  proto_handle_t *proto = proto_env_get_proto(pb);

  ULONG num = bench_get_num();
  ULONG size = bench_get_size();

  /* alloc buffer */
  UBYTE *buf = AllocVec(size, MEMF_PUBLIC);
  if(buf == NULL) {
    PutStr("\nERROR: No Memory!\n");
    return 0;
  }
  ULONG num_words = size >> 1;
  ULONG sum_size = 0;
  ULONG i;

  /* start timer */
  time_stamp_t start, end;
  timer_eclock_get(timer, &start);

  /* run loop */
  for(i=0;i<num;i++) {
    int status = func(pb, i, buf, (UWORD)num_words);
    if(status != PROTO_RET_OK) {
      Printf("\nERROR: Aborting @%lu with %ld\n", i, status);
      break;
    }
    sum_size += size * size_mult;
  }

  /* stop timer */
  timer_eclock_get(timer, &end);
  time_stamp_t delta;
  timer_eclock_delta(&end, &start, &delta);

  /* calc kbps */
  ULONG us = timer_eclock_to_us(timer, &delta);
  ULONG kbps = (sum_size * 1000UL) / us;
  if(bench_get_verbose()) {
    Printf("\nResult #%04lu: data=%lu us=%lu kbps=%lu\n", i, sum_size, us, kbps);
  }

  /* free buffer */
  FreeVec(buf);

  return kbps;
}

/* ----- bench functions ----- */

static int func_msg_write(proto_env_handle_t *pb, ULONG iter,
                          UBYTE *buffer, UWORD num_words)
{
  proto_handle_t *ph = proto_env_get_proto(pb);
  return proto_chn_msg_write(ph, 0, buffer, num_words);
}

static int func_msg_read(proto_env_handle_t *pb, ULONG iter,
                         UBYTE *buffer, UWORD num_words)
{
  proto_handle_t *ph = proto_env_get_proto(pb);
  return proto_chn_msg_read(ph, 0, buffer, num_words);
}

static int func_msg_write_read(proto_env_handle_t *pb, ULONG iter,
                               UBYTE *buffer, UWORD num_words)
{
  proto_handle_t *ph = proto_env_get_proto(pb);
  int status = proto_chn_msg_write(ph, 0, buffer, num_words);
  if(status != PROTO_RET_OK) {
    return status;
  }
  return proto_chn_msg_read(ph, 0, buffer, num_words);
}

/* ----- benchmarks ----- */

static ULONG bench_action_init(bench_def_t *def, void *user_data)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)user_data;
  proto_handle_t *ph = proto_env_get_proto(pb);

  ULONG deltas[2] = { 0,0 };
  int error = proto_action_bench(ph, PROTO_ACTION_PING, deltas);
  if(error != 0) {
    proto_err(error);
  }
  return deltas[0];
}

static ULONG bench_action_exit(bench_def_t *def, void *user_data)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)user_data;
  proto_handle_t *ph = proto_env_get_proto(pb);

  ULONG deltas[2] = { 0,0 };
  int error = proto_action_bench(ph, PROTO_ACTION_PING, deltas);
  if(error != 0) {
    proto_err(error);
  }
  return deltas[1];
}

static ULONG bench_msg_write(bench_def_t *def, void *user_data)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)user_data;
  return run_kbps_loop(pb, func_msg_write, 1);
}

static ULONG bench_msg_read(bench_def_t *def, void *user_data)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)user_data;
  return run_kbps_loop(pb, func_msg_read, 1);
}

static ULONG bench_msg_write_read(bench_def_t *def, void *user_data)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)user_data;
  return run_kbps_loop(pb, func_msg_write_read, 2);
}

/* benchmark table */

bench_def_t all_benches[] = {
  { bench_action_init, "ai", "test action command init latency (us)" },
  { bench_action_exit, "ae", "test action command exit latency (us)" },
  { bench_msg_write, "mw", "message write benchmark (Kbps)" },
  { bench_msg_read, "mr", "message read benchmark (Kbps)" },
  { bench_msg_write_read, "mwr", "message write/read benchmark (Kbps)" },
  { NULL, NULL, NULL }
};

void *bench_api_init(void)
{
  proto_env_handle_t *pb;

  int init_res;
  pb = proto_env_init((struct Library *)SysBase, &init_res);
  if(init_res == PROTO_ENV_OK) {
    return pb;
  } else {
    PutStr(proto_env_perror(init_res));
    PutStr(" -> ABORT\n");
    return NULL;
  }
}

void bench_api_exit(void *user_data)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)user_data;
  proto_env_exit(pb);
}
