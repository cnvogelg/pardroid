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

#include "pamela.h"

#include "bench.h"
#include "bench_main.h"
#include "fwid.h"

#define PORT 1000

struct bench_data {
  pamela_handle_t *ph;
  pamela_channel_t *ch;
};
typedef struct bench_data bench_data_t;

static bench_data_t data;

static void pamela_err(int res)
{
  Printf("\nPAMELA ERROR: %ld  %s\n", res, (LONG)pamela_perror(res));
}

static int wait_event(bench_data_t *data)
{
  int wait = pamela_event_wait(data->ph, 10, 0, NULL);
  if(wait == PAMELA_WAIT_EVENT) {
    int error = pamela_event_update(data->ph, NULL);
    if(error != PAMELA_OK) {
      pamela_err(error);
      return error;
    }
    return PAMELA_OK;
  } else {
    Printf("wait_event: wrong result: %ld\n", wait);
    return PAMELA_ERROR_UNKNOWN;
  }
}

/* ----- run loop helper ----- */

typedef int (*loop_func_t)(bench_data_t *data, ULONG iter,
                           UBYTE *buffer, UWORD num_words);

static ULONG run_kbps_loop(bench_data_t *data,
                           loop_func_t loop_func,
                           ULONG size_mult)
{
  timer_handle_t *timer = pamela_get_timer(data->ph);

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
    if(status != num_bytes) {
      Printf("\nERROR: Aborting @%lu with status %ld\n", i, status);
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
  int res = pamela_write_request(data->ch, buffer, num_bytes);
  if(res != PAMELA_OK) {
    return res;
  }

  res = wait_event(data);
  if(res != PAMELA_OK) {
    return res;
  }

  res = pamela_write_setup(data->ch);
  if(res != num_bytes) {
    return res;
  }

  res = pamela_write_block(data->ch);
  if(res != num_bytes) {
    return res;
  }

  return num_bytes;
}

static int loop_msg_read(bench_data_t *data, ULONG iter,
                         UBYTE *buffer, UWORD num_bytes)
{
  int res = pamela_read_request(data->ch, buffer, num_bytes);
  if(res != PAMELA_OK) {
    return res;
  }

  res = wait_event(data);
  if(res != PAMELA_OK) {
    return res;
  }

  res = pamela_read_setup(data->ch);
  if(res != num_bytes) {
    return res;
  }

  res = pamela_read_block(data->ch);
  if(res != num_bytes) {
    return res;
  }

  return num_bytes;
}

static int loop_msg_write_read(bench_data_t *data, ULONG iter,
                               UBYTE *buffer, UWORD num_bytes)
{
  int res = pamela_write_request(data->ch, buffer, num_bytes);
  if(res != PAMELA_OK) {
    return res;
  }

  res = wait_event(data);
  if(res != PAMELA_OK) {
    return res;
  }

  res = pamela_write_setup(data->ch);
  if(res != num_bytes) {
    return res;
  }

  res = pamela_write_block(data->ch);
  if(res != num_bytes) {
    return res;
  }

  res = pamela_read_request(data->ch, buffer, num_bytes);
  if(res != PAMELA_OK) {
    return res;
  }

  res = wait_event(data);
  if(res != PAMELA_OK) {
    return res;
  }

  res = pamela_read_setup(data->ch);
  if(res != num_bytes) {
    return res;
  }

  res = pamela_read_block(data->ch);
  if(res != num_bytes) {
    return res;
  }

  return num_bytes;
}

// msg + seek

static int loop_msg_write_seek(bench_data_t *data, ULONG iter,
                               UBYTE *buffer, UWORD num_bytes)
{
  int res = pamela_seek(data->ch, iter);
  if(res != PAMELA_OK) {
    return res;
  }

  res = pamela_write_request(data->ch, buffer, num_bytes);
  if(res != PAMELA_OK) {
    return res;
  }

  res = wait_event(data);
  if(res != PAMELA_OK) {
    return res;
  }

  res = pamela_write_setup(data->ch);
  if(res != num_bytes) {
    return res;
  }

  res = pamela_write_block(data->ch);
  if(res != num_bytes) {
    return res;
  }

  return num_bytes;
}

static int loop_msg_read_seek(bench_data_t *data, ULONG iter,
                              UBYTE *buffer, UWORD num_bytes)
{
  int res = pamela_seek(data->ch, iter);
  if(res != PAMELA_OK) {
    return res;
  }

  res = pamela_read_request(data->ch, buffer, num_bytes);
  if(res != PAMELA_OK) {
    return res;
  }

  res = wait_event(data);
  if(res != PAMELA_OK) {
    return res;
  }

  res = pamela_read_setup(data->ch);
  if(res != num_bytes) {
    return res;
  }

  res = pamela_read_block(data->ch);
  if(res != num_bytes) {
    return res;
  }

  return num_bytes;
}

static int loop_msg_write_read_seek(bench_data_t *data, ULONG iter,
                                    UBYTE *buffer, UWORD num_bytes)
{
  int res = pamela_seek(data->ch, iter);
  if(res != PAMELA_OK) {
    return res;
  }

  res = pamela_write_request(data->ch, buffer, num_bytes);
  if(res != PAMELA_OK) {
    return res;
  }

  res = wait_event(data);
  if(res != PAMELA_OK) {
    return res;
  }

  res = pamela_write_setup(data->ch);
  if(res != num_bytes) {
    return res;
  }

  res = pamela_write_block(data->ch);
  if(res != num_bytes) {
    return res;
  }

  res = pamela_read_request(data->ch, buffer, num_bytes);
  if(res != PAMELA_OK) {
    return res;
  }

  res = wait_event(data);
  if(res != PAMELA_OK) {
    return res;
  }

  res = pamela_read_setup(data->ch);
  if(res != num_bytes) {
    return res;
  }

  res = pamela_read_block(data->ch);
  if(res != num_bytes) {
    return res;
  }

  return num_bytes;
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

/* benchmark table */

bench_def_t all_benches[] = {
  { bench_msg_write, "mw", "message write benchmark (Kbps)" },
  { bench_msg_read, "mr", "message read benchmark (Kbps)" },
  { bench_msg_write_read, "mwr", "message write/read benchmark (Kbps)" },

  { bench_msg_write_seek, "mws", "seek + message write benchmark (Kbps)" },
  { bench_msg_read_seek, "mrs", "seek + message read benchmark (Kbps)" },
  { bench_msg_write_read_seek, "mwrs", "seek + message write/read benchmark (Kbps)" },

  { NULL, NULL, NULL }
};

void *bench_api_init(void)
{
  int init_res;
  data.ph = pamela_init((struct Library *)SysBase, &init_res);
  if(data.ph != NULL) {

    /* check firmware */
    pamela_devinfo_t devinfo;
    pamela_devinfo(data.ph, &devinfo);
    if(devinfo.firmware_id == FWID_TEST_PAMELA) {

      /* open channel */
      data.ch = pamela_open(data.ph, PORT, &init_res);
      if(data.ch != NULL) {

        // wait for open event
        int res = wait_event(&data);
        if(res == PAMELA_OK) {
          return &data;
        } else {
          Printf("error waiting for open!\n");
          pamela_exit(data.ph);
          return NULL;
        }
      } else {
        Printf("error setting up channel!\n");
        pamela_exit(data.ph);
        return NULL;
      }
    }
    Printf("wrong firmware: %04lx\n", devinfo.firmware_id);
  } else {
    PutStr("error setting up pamela!\n");
  }
  return NULL;
}

void bench_api_exit(void *user_data)
{
  bench_data_t *data = (bench_data_t *)user_data;
  
  pamela_close(data->ch);
  wait_event(data);
  pamela_exit(data->ph);
}
