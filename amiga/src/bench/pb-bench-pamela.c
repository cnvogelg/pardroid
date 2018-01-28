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
static params_t params = { 0, NULL, NULL };

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

typedef int (*loop_func_t)(proto_handle_t *ph, void *user_data);

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
    int status = func(proto, user_data);
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
  Printf("#%lu: data=%lu us=%lu kbps=%lu\n", i, sum_size, us, kbps);
}

typedef struct {
  UBYTE *buffer;
  ULONG  num_words;
} msgio_data_t;

static int func_msg_write(proto_handle_t *ph, void *user_data)
{
  msgio_data_t *data = (msgio_data_t *)user_data;
  return proto_msg_write_single(ph, 0, data->buffer, data->num_words);
}

static int func_msg_read(proto_handle_t *ph, void *user_data)
{
  msgio_data_t *data = (msgio_data_t *)user_data;
  UWORD size = data->num_words;
  return proto_msg_read_single(ph, 0, data->buffer, &size);
}

static int func_msg_write_read(proto_handle_t *ph, void *user_data)
{
  msgio_data_t *data = (msgio_data_t *)user_data;
  int status = proto_msg_write_single(ph, 0, data->buffer, data->num_words);
  if(status != PROTO_RET_OK) {
    return status;
  }
  UWORD size = data->num_words;
  return proto_msg_read_single(ph, 0, data->buffer, &size);
}

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

/* benchmark table */

static bench_def_t all_benches[] = {
  { bench_action, "action", "test action command latency" },
  { bench_msg_write, "mw", "message write benchmark" },
  { bench_msg_read, "mr", "message read benchmark" },
  { bench_msg_write_read, "mwr", "message write/read benchmark" },
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

    /* reset firmware */
    proto_handle_t *proto = pamela_get_proto(pb);
    res = proto_reset(proto, 1);
    if(res == PROTO_RET_OK) {

      /* run test */
      res = bench_main(all_benches, params.bench, pb);

    } else {
      PutStr(proto_perror(res));
      PutStr(" -> ABORT\n");
    }

    pamela_exit(pb);
  } else {
    PutStr(pamela_perror(res));
    PutStr(" -> ABORT\n");
  }

  /* Finally free args */
  FreeArgs(args);
  return res;
}
