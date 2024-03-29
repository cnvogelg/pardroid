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

#include "proto_io.h"
#include "proto_atom.h"
#include "proto/wire_dev.h"

#include "bench.h"
#include "bench_main.h"
#include "fwid.h"

#define TEST_CHANNEL  7

struct bench_data {
  proto_env_handle_t  *env;
  proto_handle_t      *proto;
};
typedef struct bench_data bench_data_t;

static bench_data_t data;

static void proto_err(int res)
{
  Printf("\nPROTO ERROR: %ld  %s\n", res, (LONG)proto_atom_perror(res));
}

/* ----- run loop helper ----- */

typedef int (*loop_func_t)(bench_data_t *data, ULONG iter,
                           UBYTE *buffer, UWORD num_words);

static ULONG run_kbps_loop(bench_data_t *data,
                           loop_func_t loop_func,
                           ULONG size_mult)
{
  timer_handle_t *timer = proto_env_get_timer(data->env);
  proto_handle_t *proto = data->proto;

  ULONG num = bench_get_num();
  ULONG num_bytes = bench_get_size();

  /* alloc buffer */
  UBYTE *buf = AllocVec(num_bytes, MEMF_PUBLIC);
  if(buf == NULL) {
    PutStr("\nERROR: No Memory!\n");
    return 0;
  }
  ULONG sum_size = 0;
  ULONG i;

  /* start timer */
  stopwatch_t watch;
  bench_time_start(timer, &watch);

  /* run loop */
  for(i=0;i<num;i++) {
    int status = loop_func(data, i, buf, (UWORD)num_bytes);
    if(status != PROTO_RET_OK) {
      Printf("\nERROR: Aborting @%lu with %ld\n", i, status);
      break;
    }
    sum_size += num_bytes * size_mult;
  }

  /* stop timer */
  bench_time_stop(timer, &watch, sum_size);
  if(bench_get_verbose()) {
    Printf("\nResult #%04lu: data=%lu us=%lu kbps=%lu\n", i, sum_size, watch.us, watch.kbps);
  }

  /* free buffer */
  FreeVec(buf);

  return watch.kbps;
}

/* ----- bench functions ----- */

// only msg

static int loop_msg_write(bench_data_t *data, ULONG iter,
                          UBYTE *buffer, UWORD num_bytes)
{
  // set write size
  int res = proto_io_write_request(data->proto, TEST_CHANNEL, num_bytes);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // write buffer
  res = proto_io_write_data(data->proto, TEST_CHANNEL, buffer, num_bytes);
  return res;
}

static int loop_msg_read(bench_data_t *data, ULONG iter,
                         UBYTE *buffer, UWORD num_bytes)
{
  // set read size
  int res = proto_io_read_request(data->proto, TEST_CHANNEL, num_bytes);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // read buffer
  res = proto_io_read_data(data->proto, TEST_CHANNEL, buffer, num_bytes);
  return res;
}

static int loop_msg_write_read(bench_data_t *data, ULONG iter,
                               UBYTE *buffer, UWORD num_bytes)
{
  // set write size
  int res = proto_io_write_request(data->proto, TEST_CHANNEL, num_bytes);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // write buffer
  res = proto_io_write_data(data->proto, TEST_CHANNEL, buffer, num_bytes);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // set read size
  res = proto_io_read_request(data->proto, TEST_CHANNEL, num_bytes);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // read buffer
  res = proto_io_read_data(data->proto, TEST_CHANNEL, buffer, num_bytes);
  return res;
}

// msg + seek

static int loop_msg_write_seek(bench_data_t *data, ULONG iter,
                               UBYTE *buffer, UWORD num_bytes)
{
  int res;

  // seek
  res = proto_io_seek(data->proto, TEST_CHANNEL, iter);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // set write size
  res = proto_io_write_request(data->proto, TEST_CHANNEL, num_bytes);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // write buffer
  res = proto_io_write_data(data->proto, TEST_CHANNEL, buffer, num_bytes);
  return res;
}

static int loop_msg_read_seek(bench_data_t *data, ULONG iter,
                              UBYTE *buffer, UWORD num_bytes)
{
  int res;

  // seek
  res = proto_io_seek(data->proto, TEST_CHANNEL, iter);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // set read size
  res = proto_io_read_request(data->proto, TEST_CHANNEL, num_bytes);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // read buffer
  res = proto_io_read_data(data->proto, TEST_CHANNEL, buffer, num_bytes);
  return res;
}

static int loop_msg_write_read_seek(bench_data_t *data, ULONG iter,
                                    UBYTE *buffer, UWORD num_bytes)
{
  int res;

  // seek
  res = proto_io_seek(data->proto, TEST_CHANNEL, iter);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // set write size
  res = proto_io_write_request(data->proto, TEST_CHANNEL, num_bytes);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // write buffer
  res = proto_io_write_data(data->proto, TEST_CHANNEL, buffer, num_bytes);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // set read size
  res = proto_io_read_request(data->proto, TEST_CHANNEL, num_bytes);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // read buffer
  res = proto_io_read_data(data->proto, TEST_CHANNEL, buffer, num_bytes);
  return res;
}

// msg + status

static int loop_msg_write_status(bench_data_t *data, ULONG iter,
                                 UBYTE *buffer, UWORD num_bytes)
{
  int res;

  // seek
  res = proto_io_seek(data->proto, TEST_CHANNEL, iter);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // set write size
  res = proto_io_write_request(data->proto, TEST_CHANNEL, num_bytes);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // get status
  UWORD status;
  res = proto_io_status(data->proto, TEST_CHANNEL, &status);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // write buffer
  res = proto_io_write_data(data->proto, TEST_CHANNEL, buffer, num_bytes);
  return res;
}

static int loop_msg_read_status(bench_data_t *data, ULONG iter,
                                UBYTE *buffer, UWORD num_bytes)
{
  int res;

  // seek
  res = proto_io_seek(data->proto, TEST_CHANNEL, iter);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // set read size
  res = proto_io_read_request(data->proto, TEST_CHANNEL, num_bytes);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // get status
  UWORD status;
  res = proto_io_status(data->proto, TEST_CHANNEL, &status);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // read buffer
  res = proto_io_read_data(data->proto, TEST_CHANNEL, buffer, num_bytes);
  return res;
}

// msg + mask + status

static int loop_msg_write_mask_status(bench_data_t *data, ULONG iter,
                                 UBYTE *buffer, UWORD num_bytes)
{
  int res;

  // seek
  res = proto_io_seek(data->proto, TEST_CHANNEL, iter);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // set write size
  res = proto_io_write_request(data->proto, TEST_CHANNEL, num_bytes);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // get event mask
  UWORD event_mask;
  res = proto_io_get_event_mask(data->proto, &event_mask);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // get status
  UWORD status;
  res = proto_io_status(data->proto, TEST_CHANNEL, &status);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // write buffer
  res = proto_io_write_data(data->proto, TEST_CHANNEL, buffer, num_bytes);
  return res;
}

static int loop_msg_read_mask_status(bench_data_t *data, ULONG iter,
                                UBYTE *buffer, UWORD num_bytes)
{
  int res;

  // seek
  res = proto_io_seek(data->proto, TEST_CHANNEL, iter);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // set read size
  res = proto_io_read_request(data->proto, TEST_CHANNEL, num_bytes);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // get event mask
  UWORD event_mask;
  res = proto_io_get_event_mask(data->proto, &event_mask);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // get status
  UWORD status;
  res = proto_io_status(data->proto, TEST_CHANNEL, &status);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // read buffer
  res = proto_io_read_data(data->proto, TEST_CHANNEL, buffer, num_bytes);
  return res;
}

/* ----- benchmarks ----- */

static ULONG bench_action_init(bench_def_t *def, void *user_data)
{
  bench_data_t *data = (bench_data_t *)user_data;

  ULONG deltas[2] = { 0,0 };
  int error = proto_atom_action_bench(data->proto, PROTO_DEV_CMD_ACTION_PING, deltas);
  if(error != 0) {
    proto_err(error);
  }
  return deltas[0];
}

static ULONG bench_action_exit(bench_def_t *def, void *user_data)
{
  bench_data_t *data = (bench_data_t *)user_data;

  ULONG deltas[2] = { 0,0 };
  int error = proto_atom_action_bench(data->proto, PROTO_DEV_CMD_ACTION_PING, deltas);
  if(error != 0) {
    proto_err(error);
  }
  return deltas[1];
}

// only msg

static ULONG bench_msg_write(bench_def_t *def, void *user_data)
{
  bench_data_t *data = (bench_data_t *)user_data;
  return run_kbps_loop(data, loop_msg_write, 1);
}

static ULONG bench_msg_read(bench_def_t *def, void *user_data)
{
  bench_data_t *data = (bench_data_t *)user_data;
  return run_kbps_loop(data, loop_msg_read, 1);
}

static ULONG bench_msg_write_read(bench_def_t *def, void *user_data)
{
  bench_data_t *data = (bench_data_t *)user_data;
  return run_kbps_loop(data, loop_msg_write_read, 2);
}

// msg + seek

static ULONG bench_msg_write_seek(bench_def_t *def, void *user_data)
{
  bench_data_t *data = (bench_data_t *)user_data;
  return run_kbps_loop(data, loop_msg_write_seek, 1);
}

static ULONG bench_msg_read_seek(bench_def_t *def, void *user_data)
{
  bench_data_t *data = (bench_data_t *)user_data;
  return run_kbps_loop(data, loop_msg_read_seek, 1);
}

static ULONG bench_msg_write_read_seek(bench_def_t *def, void *user_data)
{
  bench_data_t *data = (bench_data_t *)user_data;
  return run_kbps_loop(data, loop_msg_write_read_seek, 2);
}

// msg + status

static ULONG bench_msg_write_status(bench_def_t *def, void *user_data)
{
  bench_data_t *data = (bench_data_t *)user_data;
  return run_kbps_loop(data, loop_msg_write_status, 1);
}

static ULONG bench_msg_read_status(bench_def_t *def, void *user_data)
{
  bench_data_t *data = (bench_data_t *)user_data;
  return run_kbps_loop(data, loop_msg_read_status, 1);
}

// msg + status + mask

static ULONG bench_msg_write_mask_status(bench_def_t *def, void *user_data)
{
  bench_data_t *data = (bench_data_t *)user_data;
  return run_kbps_loop(data, loop_msg_write_mask_status, 1);
}

static ULONG bench_msg_read_mask_status(bench_def_t *def, void *user_data)
{
  bench_data_t *data = (bench_data_t *)user_data;
  return run_kbps_loop(data, loop_msg_read_mask_status, 1);
}

/* benchmark table */

bench_def_t all_benches[] = {
  { bench_action_init, "ai", "test action command init latency (us)" },
  { bench_action_exit, "ae", "test action command exit latency (us)" },
  
  { bench_msg_write, "mw", "message write benchmark (Kbps)" },
  { bench_msg_read, "mr", "message read benchmark (Kbps)" },
  { bench_msg_write_read, "mwr", "message write/read benchmark (Kbps)" },

  { bench_msg_write_seek, "mws", "seek + message write benchmark (Kbps)" },
  { bench_msg_read_seek, "mrs", "seek + message read benchmark (Kbps)" },
  { bench_msg_write_read_seek, "mwrs", "seek + message write/read benchmark (Kbps)" },

  { bench_msg_write_status, "mwt", "message write + status benchmark (Kbps)" },
  { bench_msg_read_status, "mrt", "message read + status benchmark (Kbps)" },

  { bench_msg_write_mask_status, "mwm", "message write + mask + status benchmark (Kbps)" },
  { bench_msg_read_mask_status, "mrm", "message read + mask + status benchmark (Kbps)" },

  { NULL, NULL, NULL }
};

void *bench_api_init(void)
{
  int init_res;
  data.env = proto_env_init((struct Library *)SysBase, &init_res);
  if(data.env != NULL) {
    data.proto = proto_io_init(data.env);
    if(data.proto != NULL) {

      /* check firmware */
      WORD fw_id = 0;
      proto_dev_get_fw_id(data.proto, &fw_id);
      if(fw_id == FWID_TEST_PROTO_IO) {
        return &data;
      }
      Printf("wrong firmware: %04lx\n", fw_id);
    } else {
      PutStr("error setting up proto!\n");
    }
  } else {
    PutStr("error setting up env!\n");
  }
  return NULL;
}

void bench_api_exit(void *user_data)
{
  bench_data_t *data = (bench_data_t *)user_data;
  
  proto_io_exit(data->proto);
  proto_env_exit(data->env);
}
