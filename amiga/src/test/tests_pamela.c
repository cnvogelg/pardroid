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

#include "fwid.h"
#include "test.h"

static UWORD test_size;
static UWORD test_bias;
static UWORD test_add_size;
static UWORD test_sub_size;
static UBYTE test_channel;

void tests_proto_config(UWORD size, UWORD bias, UWORD add_size, UWORD sub_size,
                        UBYTE channel)
{
  test_size = size;
  test_bias = bias;
  test_add_size = add_size;
  test_sub_size = sub_size;
  test_channel = channel;
}

int test_reset(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);

  /* perform reset */
  int res = proto_reset(proto, 1);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "reset";
    return res;
  }

  /* re-attach to device after reset */
  res = proto_action(proto, PROTO_ACTION_ATTACH);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "attach";
    return res;
  }

  return 0;
}

int test_ping(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);

  int res = proto_action(proto, PROTO_ACTION_PING);
  if(res == 0) {
    return 0;
  } else {
    p->error = proto_perror(res);
    p->section = "ping";
    return res;
  }
}

int test_func_write(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  UWORD v = 0x4711;

  int res = proto_function_write(proto, PROTO_FUNC_REGADDR_SET, v);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }
  return 0;
}

int test_func_read(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  UWORD v;

  int res = proto_function_read(proto, PROTO_FUNC_REGADDR_GET, &v);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }
  return 0;
}

int test_func_write_read(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  UWORD v = (UWORD)p->iter + test_bias;

  /* write */
  int res = proto_function_write(proto, PROTO_FUNC_REGADDR_SET, v);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  /* read back */
  UWORD r;
  res = proto_function_read(proto, PROTO_FUNC_REGADDR_GET, &r);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  /* check */
  if(v != r) {
    p->error = "value mismatch";
    p->section = "compare";
    sprintf(p->extra, "w=%04x r=%04x", v, r);
    return 1;
  }

  return 0;
}

int test_offset_write_read(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);

  UWORD v = (UWORD)p->iter + test_bias;
  v %= PROTO_MAX_CHANNEL;

  ULONG val = 0xdeadbeef + v;

  /* write offset */
  int res = offset_set(proto, v, val);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "set";
    return res;
  }

  /* read offset */
  ULONG off;
  res = offset_get(proto, v, &off);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "get";
    return res;
  }

  /* compare */
  if(val != off) {
    p->error = "value mismatch";
    p->section = "compare";
    sprintf(p->extra, "w=%08lx, r=%08lx", val, off);
    return 1;
  }

  return 0;
}

int test_msg_empty(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);

  int res = proto_msg_write_single(proto, test_channel, 0, 0);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  UWORD size = 0;
  res = proto_msg_read_single(proto, test_channel, 0, &size);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  if(size != 0) {
    p->error = "not empty";
    p->section = "compare";
    sprintf(p->extra, "%04x", size);
    return 1;
  }

  return 0;
}

int test_msg_tiny(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);

  ULONG data = 0xdeadbeef;
  int res = proto_msg_write_single(proto, test_channel, (UBYTE *)&data, 2);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  UWORD size = 2;
  res = proto_msg_read_single(proto, test_channel, (UBYTE *)&data, &size);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  if(size != 2) {
    p->error = "not two words";
    p->section = "compare";
    sprintf(p->extra, "%04x", size);
    return 1;
  }

  if(data != 0xdeadbeef) {
    p->error = "invalid value";
    p->section = "compare";
    sprintf(p->extra, "%08lx", data);
    return 1;
  }

  return 0;
}

static ULONG get_default_size(void)
{
  /* get buffer size */
  ULONG size = 256;
  if(test_size != 0) {
    size = test_size;
    if(size < 2) {
      size = 2;
    }
    if(size & 1 == 1) {
      size++;
    }
  }
  return size;
}

static UBYTE get_start_byte(void)
{
  /* set start byte value */
  UBYTE start = 0;
  if(test_bias != 0) {
    start = (UBYTE)test_bias;
  }
  return start;
}

static ULONG get_size(ULONG size)
{
  if(test_add_size != 0) {
    size += test_add_size;
  }
  if(test_sub_size != 0) {
    size -= test_sub_size;
  }
  return size;
}

static void fill_buffer(ULONG size, UBYTE *mem)
{
  /* fill buffer */
  UBYTE data = get_start_byte();
  for(ULONG i=0;i<size;i++) {
    mem[i] = data++;
  }
}

static int validate_buffer(ULONG size, UBYTE *mem)
{
  UBYTE data = get_start_byte();
  for(ULONG i=0;i<size;i++) {
    if(mem[i] != data) {
      return 1;
    }
    data++;
  }
  return 0;
}

static ULONG check_buffer(ULONG size, UBYTE *mem1, UBYTE *mem2)
{
  /* check buf */
  ULONG result = size;
  for(ULONG i=0;i<size;i++) {
    if(mem1[i] != mem2[i]) {
      result = i;
      break;
    }
  }
  return result;
}

static int msg_read_write(test_t *t, test_param_t *p, ULONG size)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);

  UBYTE *mem_w = AllocVec(size, MEMF_PUBLIC);
  if(mem_w == 0) {
    p->error = "out of mem";
    p->section = "init";
    return 1;
  }

  ULONG size_r = get_size(size);
  BYTE *mem_r = AllocVec(size_r, MEMF_PUBLIC);
  if(mem_r == 0) {
    FreeVec(mem_w);
    p->error = "out of mem";
    p->section = "init";
    return 1;
  }

  fill_buffer(size, mem_w);

  UWORD words = size>>1;

  /* send buffer */
  int res = proto_msg_write_single(proto, test_channel, mem_w, words);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  /* receive buffer */
  UWORD got_words = size_r>>1;
  res = proto_msg_read_single(proto, test_channel, mem_r, &got_words);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  /* check buf size */
  if(got_words != words) {
    p->error = "size mismatch";
    p->section = "compare";
    sprintf(p->extra, "w=%04x r=%04x", words, got_words);
    return 1;
  }

  /* check buf */
  ULONG pos = check_buffer(size, mem_w, mem_r);
  if(pos < size) {
    p->error = "value mismatch";
    p->section = "compare";
    sprintf(p->extra, "@%08lx: w=%02x r=%02x", pos, (UWORD)mem_w[pos], (UWORD)mem_r[pos]);
    return 1;
  }

  FreeVec(mem_w);
  FreeVec(mem_r);
  return 0;
}

int test_msg_size(test_t *t, test_param_t *p)
{
  ULONG size = get_default_size();
  return msg_read_write(t, p, size);
}

#define REG_MAX_BYTES (PROTO_REGOFFSET_USER + 2)

int test_msg_size_max(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  UWORD max_bytes;

  /* read max size from firmware */
  int res = reg_get(proto, REG_MAX_BYTES, &max_bytes);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read max_bytes";
    return res;
  }

  return msg_read_write(t, p, max_bytes);
}

int test_msg_size_chunks(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);

  ULONG size = get_default_size();
  if(size < 4) {
    size = 4;
  }

  UBYTE *mem_w = AllocVec(size, MEMF_PUBLIC);
  if(mem_w == 0) {
    p->error = "out of mem";
    p->section = "init";
    return 1;
  }

  ULONG size_r = get_size(size);
  BYTE *mem_r = AllocVec(size_r, MEMF_PUBLIC);
  if(mem_r == 0) {
    FreeVec(mem_w);
    p->error = "out of mem";
    p->section = "init";
    return 1;
  }

  fill_buffer(size, mem_w);

  ULONG words = size>>1;
  ULONG c1_words = words>>1;
  ULONG c2_words = words - c1_words;
  UBYTE *c1_buf = mem_w;
  UBYTE *c2_buf = mem_w + (c1_words << 1);

  /* send buffer */
  proto_iov_node_t part2_w = {
    c2_words,
    c2_buf,
    0
  };
  proto_iov_t msgiov_w = {
    words,
    0xdead, /* extra */
    c1_words,
    c1_buf,
    &part2_w
  };
  int res = proto_msg_write(proto, test_channel, &msgiov_w);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  ULONG words_r = size_r>>1;
  c1_buf = mem_r;
  c2_buf = mem_r + (c1_words << 1);
  c2_words = words_r - c1_words;

  /* receive buffer */
  proto_iov_node_t part2_r = {
    c2_words,
    c2_buf,
    0
  };
  proto_iov_t msgiov_r = {
    words_r,
    0,
    c1_words,
    c1_buf,
    &part2_r
  };
  res = proto_msg_read(proto, test_channel, &msgiov_r);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }
  UWORD got_words = (UWORD)(msgiov_r.total_words & 0xffff);

  /* check buf size */
  if(got_words != words) {
    p->error = "size mismatch";
    p->section = "compare";
    sprintf(p->extra, "w=%04lx r=%04x", words, msgiov_r.total_words);
    return 1;
  }

  /* check buf */
  ULONG pos = check_buffer(size, mem_w, mem_r);
  if(pos < size) {
    p->error = "value mismatch";
    p->section = "compare";
    sprintf(p->extra, "@%08lx: w=%02x r=%02x", pos, (UWORD)mem_w[pos], (UWORD)mem_r[pos]);
    return 1;
  }

  /* check extra */
  if(msgiov_w.extra != msgiov_r.extra) {
    p->error = "extra mismatch";
    p->section = "compare";
    sprintf(p->extra, "w=%04x r=%04x", msgiov_w.extra, msgiov_r.extra);
    return 1;
  }

  FreeVec(mem_w);
  FreeVec(mem_r);
  return 0;
}

int test_msg_write(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  ULONG size = get_default_size();

  UBYTE *mem_w = AllocVec(size, MEMF_PUBLIC);
  if(mem_w == 0) {
    p->error = "out of mem";
    p->section = "init";
    return 1;
  }

  fill_buffer(size, mem_w);

  UWORD words = size>>1;

  /* send buffer */
  int res = proto_msg_write_single(proto, test_channel, mem_w, words);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  FreeVec(mem_w);
  return 0;
}

int test_msg_write_too_large(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);

  UWORD size;

  /* read max size from firmware */
  int res = reg_get(proto, REG_MAX_BYTES, &size);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read max_bytes";
    return res;
  }

  /* too many now */
  size += 2;

  UBYTE *mem_w = AllocVec(size, MEMF_PUBLIC);
  if(mem_w == 0) {
    p->error = "out of mem";
    p->section = "init";
    return 1;
  }

  fill_buffer(size, mem_w);

  UWORD words = size>>1;

  /* send buffer and expect msg to large */
  res = proto_msg_write_single(proto, test_channel, mem_w, words);
  if(res != PROTO_RET_MSG_TOO_LARGE) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  /* read max size from firmware */
  res = reg_get(proto, REG_MAX_BYTES, &size);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read max_bytes again";
    return res;
  }

  FreeVec(mem_w);
  return 0;
}

int test_msg_read(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  ULONG size = get_default_size();
  ULONG size_r = get_size(size);

  BYTE *mem_r = AllocVec(size_r, MEMF_PUBLIC);
  if(mem_r == 0) {
    p->error = "out of mem";
    p->section = "init";
    return 1;
  }

  UWORD words = size>>1;

  /* receive buffer */
  UWORD got_words = size_r>>1;
  int res = proto_msg_read_single(proto, test_channel, mem_r, &got_words);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  FreeVec(mem_r);
  return 0;
}

int test_msg_read_too_large(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);

  UWORD size;

  /* read max size from firmware */
  int res = reg_get(proto, REG_MAX_BYTES, &size);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read max_bytes";
    return res;
  }

  /* too many now */
  size += 2;

  BYTE *mem_r = AllocVec(size, MEMF_PUBLIC);
  if(mem_r == 0) {
    p->error = "out of mem";
    p->section = "init";
    return 1;
  }

  UWORD words = size>>1;

  /* receive buffer */
  res = proto_msg_read_single(proto, test_channel, mem_r, &words);
  if(res != PROTO_RET_MSG_TOO_LARGE) {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  FreeVec(mem_r);
  return 0;
}

/* ---------- status tests ----------------------------------------------- */

#define REG_SIM_PENDING (PROTO_REGOFFSET_USER + 5)
#define REG_SIM_EVENT   (PROTO_REGOFFSET_USER + 6)
#define REG_TEST_MODE   (PROTO_REGOFFSET_USER + 7)
#define REG_SIM_BUSY    (PROTO_REGOFFSET_USER + 9)

#define SIM_PENDING_SET   0x80

#define WAIT_S      0UL
#define WAIT_US     10000UL

#define TEST_MODE_NORMAL  0
#define TEST_MODE_ECHO    1

static int assert_timer_mask(test_param_t *p, const char *section, pamela_handle_t *pb, ULONG got)
{
  ULONG timer_mask = pamela_get_timer_sigmask(pb);

  if(got != timer_mask) {
    p->error = "no timer triggered";
    p->section = section;
    sprintf(p->extra, "got=%08lx want=%08lx", got, timer_mask);
    return 1;
  }

  return 0;
}

static int assert_event_mask(test_param_t *p, const char *section,
                             pamela_handle_t *pb, ULONG got)
{
  ULONG event_mask = pamela_get_event_sigmask(pb);

  if(got != event_mask) {
    p->error = "no event triggered";
    p->section = section;
    sprintf(p->extra, "got=%08lx want=%08lx", got, event_mask);
    return 1;
  }

  return 0;
}

static int assert_num_events(test_param_t *p, const char *section,
                             pamela_handle_t *pb,
                             UWORD num_events, UWORD num_signals)
{
  UWORD got_events = pamela_get_num_events(pb);
  UWORD got_signals = pamela_get_num_event_signals(pb);

  /* check number of events aka irqs */
  if(num_events != got_events) {
    p->error = "num events (ack irqs) mismatch";
    p->section = section;
    sprintf(p->extra, "got=%u want=%u", got_events, num_events);
    return 1;
  }

  /* check number of signals */
  if(num_signals != got_signals) {
    p->error = "num signals mismatch";
    p->section = section;
    sprintf(p->extra, "got=%u want=%u", got_signals, num_signals);
    return 1;
  }

  return 0;
}

static int run_with_events(test_t *t, test_param_t *p, test_func_t func)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;

  /* init events */
  int res = pamela_init_events(pb);
  if(res != PAMELA_OK) {
    p->error = pamela_perror(res);
    p->section = "pamela_init_events";
    return res;
  }

  /* call test func */
  res = func(t, p);

  /* cleanup events */
  pamela_exit_events(pb);

  return res;
}

int run_status_timer_sig(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;

  /* wait for either timeout or ack */
  ULONG got = pamela_wait_event(pb, WAIT_S, WAIT_US, 0);

  int res = assert_timer_mask(p, "main", pb, got);
  if(res != 0) {
    return res;
  }

  return assert_num_events(p, "main", pb, 0, 0);
}

int test_status_timer_sig(test_t *t, test_param_t *p)
{
  return run_with_events(t, p, run_status_timer_sig);
}

int run_status_reset_event_sig(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  status_data_t *status = pamela_get_status(pb);

  /* perform reset */
  int res = proto_reset(proto, 1);
  if(res != PROTO_RET_OK) {
    p->error = proto_perror(res);
    p->section = "reset";
    return res;
  }

  /* wait for either timeout or ack */
  ULONG got = pamela_wait_event(pb, WAIT_S, WAIT_US, 0);

  /* expect status event after reset */
  res = assert_event_mask(p, "main", pb, got);
  if(res != 0) {
    return res;
  }

  /* assume detached */
  if(status->flags != STATUS_FLAGS_DETACHED) {
    p->error = "status not detached";
    p->section = "reset";
    return 1;
  }

  /* attach again */
  res = proto_action(proto, PROTO_ACTION_ATTACH);
  if(res != PROTO_RET_OK) {
    p->error = proto_perror(res);
    p->section = "attach";
    return res;
  }

  /* wait for timeout */
  ULONG got2 = pamela_wait_event(pb, WAIT_S, WAIT_US, 0);

  /* expect status event after reset */
  res = assert_timer_mask(p, "attach", pb, got2);
  if(res != 0) {
    return res;
  }

  /* assume clear state */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not empt";
    p->section = "after";
    return 1;
  }

  return 0;
}

int test_status_reset_event_sig(test_t *t, test_param_t *p)
{
  return run_with_events(t, p, run_status_reset_event_sig);
}

int test_status_read_pending(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);

  /* update status */
  status_data_t *status = pamela_update_status(pb);

  /* assume no flags set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not init";
    p->section = "pre";
    return 1;
  }

  UWORD channel = (p->iter + test_bias) & 7;

  /* sim_pending */
  int res = reg_set(proto, REG_SIM_PENDING, SIM_PENDING_SET | channel);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "sim_pending #1";
    return res;
  }

  /* update status */
  pamela_update_status(pb);

  /* assume pending is set */
  if(status->flags != STATUS_FLAGS_PENDING) {
    p->error = "status not pending";
    p->section = "main";
    return 1;
  }

  /* check that channel is set */
  if(status->pending_channel != channel) {
    p->error = "wrong channel in status";
    p->section = "status";
    sprintf(p->extra, "got=%02x want=%02x", (UWORD)status->pending_channel, channel);
    return 1;
  }

  /* sim_pending with no channel */
  res = reg_set(proto, REG_SIM_PENDING, channel);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "sim_pending #2";
    return res;
  }

  /* update status */
  pamela_update_status(pb);

  /* assume pending is cleared again */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not init";
    p->section = "post";
    return 1;
  }

  return 0;
}

static int set_read_pending(test_param_t *p, const char *section, pamela_handle_t *pb,
                            u08 channel, u08 expect_channel, u08 irq)
{
  proto_handle_t *proto = pamela_get_proto(pb);
  status_data_t *status = pamela_get_status(pb);

  /* sim pending */
  int res = reg_set(proto, REG_SIM_PENDING, SIM_PENDING_SET | channel);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = section;
    return res;
  }

  /* wait for either timeout or event */
  ULONG got = pamela_wait_event(pb, WAIT_S, WAIT_US, 0);

  if(irq) {
    /* assume it was an event */
    res = assert_event_mask(p, section, pb, got);
    if(res != 0) {
      return res;
    }
  } else {
   /* assume it was a timeout */
    res = assert_timer_mask(p, section, pb, got);
    if(res != 0) {
      return res;
    }
  }

  /* assume pending is set */
  if(status->flags != STATUS_FLAGS_PENDING) {
    p->error = "status not pending";
    p->section = section;
    return 1;
  }

  /* check that channel is set */
  if(status->pending_channel != expect_channel) {
    p->error = "wrong channel in status";
    p->section = section;
    sprintf(p->extra, "got=%02x want=%02x", (UWORD)status->pending_channel, expect_channel);
    return 1;
  }

  return 0;
}

static int clear_read_pending(test_param_t *p, const char *section, pamela_handle_t *pb,
                            u08 channel, u08 expect_channel, u08 irq)
{
  proto_handle_t *proto = pamela_get_proto(pb);
  status_data_t *status = pamela_get_status(pb);

  /* sim pending */
  int res = reg_set(proto, REG_SIM_PENDING, channel);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = section;
    return res;
  }

  /* wait for either timeout or event */
  ULONG got = pamela_wait_event(pb, WAIT_S, WAIT_US, 0);

  // no channel expected anymore
  if(expect_channel == 0xff) {
    /* assume pending is cleared again */
    if(status->flags != STATUS_FLAGS_NONE) {
      p->error = "status not cleared";
      p->section = section;
      return 1;
    }
  } else {
    /* assume pending is set */
    if(status->flags != STATUS_FLAGS_PENDING) {
      p->error = "status not pending";
      p->section = section;
      return 1;
    }

    /* check that channel is set */
    if(status->pending_channel != expect_channel) {
      p->error = "wrong channel in status";
      p->section = section;
      sprintf(p->extra, "got=%02x want=%02x", (UWORD)status->pending_channel, expect_channel);
      return 1;
    }
  }

  if(irq) {
    /* assume it was an event */
    res = assert_event_mask(p, section, pb, got);
    if(res != 0) {
      return res;
    }
  } else {
    /* assume it was a timeout */
    res = assert_timer_mask(p, section, pb, got);
    if(res != 0) {
      return res;
    }
  }

  return 0;
}

int run_status_read_pending_sig(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  status_data_t *status = pamela_get_status(pb);

  /* assume no flags set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not init";
    p->section = "pre";
    return 1;
  }

  UWORD channel = (p->iter + test_bias) & 7;

  // set pending channel
  int res = set_read_pending(p, "set", pb, channel, channel, 1);
  if(res != 0) {
    return res;
  }

  // clear pending channel
  res = clear_read_pending(p, "clear", pb, channel, 0xff, 0);
  if(res != 0) {
    return res;
  }

  return assert_num_events(p, "main", pb, 1, 1);
}

int test_status_read_pending_sig(test_t *t, test_param_t *p)
{
  return run_with_events(t, p, run_status_read_pending_sig);
}

int run_status_read_pending_two(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  status_data_t *status = pamela_get_status(pb);

  /* assume no flags set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not init";
    p->section = "pre";
    return 1;
  }

  // set pending channel #0
  int res = set_read_pending(p, "set#0", pb, 0, 0, 1);
  if(res != 0) {
    return res;
  }

  // set pending channel #1 (#0 still active)
  res = set_read_pending(p, "set#1", pb, 1, 0, 0);
  if(res != 0) {
    return res;
  }

  // clear pending channel #0 (#1 gets active)
  res = clear_read_pending(p, "clear#0", pb, 0, 1, 1);
  if(res != 0) {
    return res;
  }

  // clear pending channel #1 (none active)
  res = clear_read_pending(p, "clear#1", pb, 1, 0xff, 0);
  if(res != 0) {
    return res;
  }

  return 0;
}

int test_status_read_pending_two(test_t *t, test_param_t *p)
{
  return run_with_events(t, p, run_status_read_pending_two);
}

int run_status_read_pending_refresh(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  status_data_t *status = pamela_get_status(pb);

  /* assume no flags set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not init";
    p->section = "pre";
    return 1;
  }

  // set pending channel #0
  int res = set_read_pending(p, "set#0", pb, 0, 0, 1);
  if(res != 0) {
    return res;
  }

  // set pending channel #0 again (refresh)
  res = set_read_pending(p, "refresh#0", pb, 0, 0, 1);
  if(res != 0) {
    return res;
  }

  // clear pending channel #0 (none active)
  res = clear_read_pending(p, "clear#0", pb, 0, 0xff, 0);
  if(res != 0) {
    return res;
  }

  return 0;
}

int test_status_read_pending_refresh(test_t *t, test_param_t *p)
{
  return run_with_events(t, p, run_status_read_pending_refresh);
}

int run_status_read_pending_refresh_active(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  status_data_t *status = pamela_get_status(pb);

  /* assume no flags set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not init";
    p->section = "pre";
    return 1;
  }

  // set pending channel #0
  int res = set_read_pending(p, "set#0", pb, 0, 0, 1);
  if(res != 0) {
    return res;
  }

  // set pending channel #1 (#0 still active)
  res = set_read_pending(p, "set#1", pb, 1, 0, 0);
  if(res != 0) {
    return res;
  }

  // set pending channel #0 again (refresh) -> #1 gets active
  res = set_read_pending(p, "refresh#0", pb, 0, 1, 1);
  if(res != 0) {
    return res;
  }

  // clear pending channel #1 (#0 active)
  res = clear_read_pending(p, "clear#1", pb, 1, 0, 1);
  if(res != 0) {
    return res;
  }

  // clear pending channel #0 (none active)
  res = clear_read_pending(p, "clear#0", pb, 0, 0xff, 0);
  if(res != 0) {
    return res;
  }

  return 0;
}

int test_status_read_pending_refresh_active(test_t *t, test_param_t *p)
{
  return run_with_events(t, p, run_status_read_pending_refresh_active);
}

int run_status_read_pending_refresh_inactive(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  status_data_t *status = pamela_get_status(pb);

  /* assume no flags set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not init";
    p->section = "pre";
    return 1;
  }

  // set pending channel #0
  int res = set_read_pending(p, "set#0", pb, 0, 0, 1);
  if(res != 0) {
    return res;
  }

  // set pending channel #1 (#0 still active)
  res = set_read_pending(p, "set#1", pb, 1, 0, 0);
  if(res != 0) {
    return res;
  }

  // set pending channel #1 again (refresh) -> #0 stays active
  res = set_read_pending(p, "refresh#0", pb, 1, 0, 0);
  if(res != 0) {
    return res;
  }

  // clear pending channel #1 (#0 active)
  res = clear_read_pending(p, "clear#1", pb, 1, 0, 0);
  if(res != 0) {
    return res;
  }

  // clear pending channel #0 (none active)
  res = clear_read_pending(p, "clear#0", pb, 0, 0xff, 0);
  if(res != 0) {
    return res;
  }

  return 0;
}

int test_status_read_pending_refresh_inactive(test_t *t, test_param_t *p)
{
  return run_with_events(t, p, run_status_read_pending_refresh_inactive);
}

int test_status_events(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);

  /* update status */
  status_data_t *status = pamela_update_status(pb);

  /* assume no flags set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not init";
    p->section = "pre";
    return 1;
  }

  /* create an event */
  UBYTE channel = (UBYTE)(p->iter + test_bias);
  channel &= 7;

  /* simulate an event */
  int res = reg_set(proto, REG_SIM_EVENT, channel);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "sim_event #1";
    return res;
  }

  /* update status: consume event and read event mask */
  pamela_update_status(pb);

  /* assume event is set */
  if(status->flags != STATUS_FLAGS_EVENTS) {
    p->error = "status has no events";
    p->section = "main";
    return 1;
  }

  /* check if correct event mask was returned */
  UBYTE mask = 1 << channel;
  if(status->event_mask != mask) {
    p->error = "wrong event mask returned";
    p->section = "main";
    sprintf(p->extra, "got=%02x want=%02x", mask, status->event_mask);
    return 1;
  }

  /* read event register and assume it to be cleared */
  UWORD val;
  res = reg_base_get_event_mask(proto, &val);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "sim_event #2";
    return res;
  }
  if(val != 0) {
    p->error = "event mask not cleared";
    p->section = "after";
    return 1;
  }

  /* update status again. no event */
  pamela_update_status(pb);

  /* assume error is not set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not init";
    p->section = "post";
    return 1;
  }

  return 0;
}

static int set_event(test_param_t *p, const char *section, pamela_handle_t *pb,
                     u08 channel)
{
  proto_handle_t *proto = pamela_get_proto(pb);
  status_data_t *status = pamela_get_status(pb);

  /* simulate an event */
  int res = reg_set(proto, REG_SIM_EVENT, channel);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = section;
    return res;
  }

  /* wait for either timeout or ack */
  ULONG got = pamela_wait_event(pb, WAIT_S, WAIT_US, 0);

  /* assume event */
  res = assert_event_mask(p, section, pb, got);
  if(res != 0) {
    return res;
  }

  /* assume event is set */
  if(status->flags != STATUS_FLAGS_EVENTS) {
    p->error = "status has no events";
    p->section = section;
    return 1;
  }

  /* check if correct event mask was returned */
  UBYTE mask = 1 << channel;
  if(status->event_mask != mask) {
    p->error = "wrong event mask returned";
    p->section = section;
    sprintf(p->extra, "got=%02x want=%02x", mask, status->event_mask);
    return 1;
  }

  return 0;
}

int run_status_events_sig(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  status_data_t *status = pamela_get_status(pb);

  /* assume no flags set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not init";
    p->section = "pre";
    return 1;
  }

  /* select a channel */
  UBYTE channel = (UBYTE)(p->iter + test_bias);
  channel &= 7;

  /* set event */
  int res = set_event(p, "set_event", pb, channel);
  if(res != 0) {
    return res;
  }

  /* read event register and assume it to be cleared */
  UWORD val;
  res = reg_base_get_event_mask(proto, &val);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "sim_event #2";
    return res;
  }
  if(val != 0) {
    p->error = "event mask not cleared";
    p->section = "after";
    return 1;
  }

  /* wait again and assume timeout, no more events */
  ULONG got2 = pamela_wait_event(pb, WAIT_S, WAIT_US, 0);

  /* assume error is not set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not init";
    p->section = "post";
    return 1;
  }

  res = assert_timer_mask(p, "post", pb, got2);
  if(res != 0) {
    return res;
  }

  return assert_num_events(p, "main", pb, 1, 1);
}

int test_status_events_sig(test_t *t, test_param_t *p)
{
  return run_with_events(t, p, run_status_events_sig);
}

int run_status_events_in_pending(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  status_data_t *status = pamela_get_status(pb);

  /* assume no flags set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not init";
    p->section = "pre";
    return 1;
  }

  /* select a channel */
  UBYTE channel = (UBYTE)(p->iter + test_bias);
  channel &= 7;

  // set pending channel #0
  int res = set_read_pending(p, "set#0", pb, channel, channel, 1);
  if(res != 0) {
    return res;
  }

  /* set event */
  res = set_event(p, "event#0", pb, channel);
  if(res != 0) {
    return res;
  }

  /* event that pending is active again */
  ULONG got = pamela_wait_event(pb, WAIT_S, WAIT_US, 0);

  /* assume event */
  res = assert_event_mask(p, "post", pb, got);
  if(res != 0) {
    return res;
  }

  /* assume pending is set again after event */
  if(status->flags != STATUS_FLAGS_PENDING) {
    p->error = "status not pending again";
    p->section = "post";
    return 1;
  }

  // clear pending channel #0 (none active)
  res = clear_read_pending(p, "clear#0", pb, 0, 0xff, 0);
  if(res != 0) {
    return res;
  }

  return 0;
}

int test_status_events_in_pending(test_t *t, test_param_t *p)
{
  return run_with_events(t, p, run_status_events_in_pending);
}

int test_status_attach_detach(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);

  /* update status */
  status_data_t *status = pamela_update_status(pb);

  /* assume no flags set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not empty";
    p->section = "pre";
    return 1;
  }

  /* detach device */
  int res = proto_action(proto, PROTO_ACTION_DETACH);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "detach failed";
    return res;
  }

  /* update status */
  pamela_update_status(pb);

  /* assume attached flag set */
  if(status->flags != STATUS_FLAGS_DETACHED) {
    p->error = "status not detached";
    p->section = "main";
    return 1;
  }

  /* attach device */
  res = proto_action(proto, PROTO_ACTION_ATTACH);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "attach failed";
    return res;
  }

  /* update status */
  pamela_update_status(pb);

  /* assume error is not set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not empty";
    p->section = "post";
    return 1;
  }

  return 0;
}

int run_status_attach_detach_sig(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  status_data_t *status = pamela_get_status(pb);

  /* assume no flags set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not empty";
    p->section = "pre";
    return 1;
  }

  /* detach device */
  int res = proto_action(proto, PROTO_ACTION_DETACH);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "detach failed";
    return res;
  }

  /* wait for either timeout or ack */
  ULONG got = pamela_wait_event(pb, WAIT_S, WAIT_US, 0);

  /* assume detached flag set */
  if(status->flags != STATUS_FLAGS_DETACHED) {
    p->error = "status not detached";
    p->section = "main";
    return 1;
  }

  /* attach device */
  res = proto_action(proto, PROTO_ACTION_ATTACH);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "detach failed";
    return res;
  }

  /* wait again and timeout */
  ULONG got2 = pamela_wait_event(pb, WAIT_S, WAIT_US, 0);

  /* assume error is not set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not empty";
    p->section = "post";
    return 1;
  }

  /* detach triggers an event */
  res = assert_event_mask(p, "main", pb, got);
  if(res != 0) {
    return res;
  }

  /* attach triggers no event */
  res = assert_timer_mask(p, "post", pb, got2);
  if(res != 0) {
    return res;
  }

  return assert_num_events(p, "main", pb, 1, 1);
}

int test_status_attach_detach_sig(test_t *t, test_param_t *p)
{
  return run_with_events(t, p, run_status_attach_detach_sig);
}

int run_status_attach_reset_sig(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  status_data_t *status = pamela_get_status(pb);

  /* assume no flags set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not empty";
    p->section = "pre";
    return 1;
  }

  /* now reset device */
  int res = proto_reset(proto, 1);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "detach failed";
    return res;
  }

  /* wait for either timeout or ack */
  ULONG got = pamela_wait_event(pb, WAIT_S, WAIT_US, 0);

  /* assume event after reset */
  res = assert_event_mask(p, "main", pb, got);
  if(res != 0) {
    return res;
  }

  /* assume detached */
  if(status->flags != STATUS_FLAGS_DETACHED) {
    p->error = "status not detached";
    p->section = "reset";
    return 1;
  }

  /* attach device */
  res = proto_action(proto, PROTO_ACTION_ATTACH);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "attach failed";
    return res;
  }

  /* wait again: no more events */
  ULONG got2 = pamela_wait_event(pb, WAIT_S, WAIT_US, 0);

  /* assume no event after attach */
  res = assert_timer_mask(p, "post", pb, got2);
  if(res != 0) {
    return res;
  }

  return 0;
}

int test_status_attach_reset_sig(test_t *t, test_param_t *p)
{
  return run_with_events(t, p, run_status_attach_reset_sig);
}

int test_base_regs(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);

  UWORD fw_version, machtag, fw_id, event;

  /* get fw_version */
  int res = reg_base_get_fw_version(proto, &fw_version);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "get_fw_version failed";
    return res;
  }

  /* get machtag */
  res = reg_base_get_fw_machtag(proto, &machtag);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "get_fw_machtag failed";
    return res;
  }

  /* get fw_id */
  res = reg_base_get_fw_id(proto, &fw_id);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "get_fw_id failed";
    return res;
  }

  /* get event */
  res = reg_base_get_event_mask(proto, &event);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "get_error failed";
    return res;
  }

  /* check fw_id */
  if(fw_id != FWID_TEST_PAMELA) {
    p->error = "no pamela test firmware!";
    p->section = "check";
    return 1;
  }

  /* assume no event */
  if(event != 0) {
    p->error = "device error set!";
    p->section = "check";
    return 1;
  }

  return 0;
}

int run_status_busy(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  status_data_t *status = pamela_get_status(pb);

  /* assume no flags set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not empty";
    p->section = "pre";
    return 1;
  }

  /* trigger busy: 100ms */
  int res = reg_set(proto, REG_SIM_BUSY, 100);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "sim_busy failed";
    return res;
  }

  /* wait for busy event */
  ULONG got = pamela_wait_event(pb, WAIT_S, WAIT_US, 0);

  /* assume event */
  res = assert_event_mask(p, "main", pb, got);
  if(res != 0) {
    return res;
  }

  /* assume busy flag */
  if(status->flags != STATUS_FLAGS_BUSY) {
    p->error = "status not busy";
    p->section = "sim_busy";
    return 1;
  }

  /* wait again: busy gone event */
  ULONG got2 = pamela_wait_event(pb, 1, 0, 0);

  /* assume no event after attach */
  res = assert_event_mask(p, "post", pb, got2);
  if(res != 0) {
    return res;
  }

  /* assume no flag */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not empty";
    p->section = "post";
    return 1;
  }

  return 0;
}

int test_status_busy(test_t *t, test_param_t *p)
{
  return run_with_events(t, p, run_status_busy);
}

// ---------- test modes ------------------------------------------

static int set_test_mode(test_param_t *p, const char *section,
                          pamela_handle_t *pb, UBYTE mode)
{
  proto_handle_t *proto = pamela_get_proto(pb);

  /* sim_pending */
  int res = reg_set(proto, REG_TEST_MODE, mode);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = section;
    return res;
  }

  return 0;
}

static int run_in_test_mode(test_t *t, test_param_t *p, test_func_t func,
                     UBYTE mode)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;

  /* set echo test mode */
  int res = set_test_mode(p, "init", pb, mode);
  if(res != 0) {
    return res;
  }

  res = run_with_events(t, p, func);

  /* set normal mode */
  set_test_mode(p, "exit", pb, TEST_MODE_NORMAL);

  return 0;
}

// ----- ECHO -----

#define MSG_SIZE  512

int run_echo_single(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  status_data_t *status = pamela_get_status(pb);

  /* alloc buffer */
  ULONG size = get_default_size();
  UBYTE *buf = AllocVec(size, MEMF_PUBLIC);
  if(buf == 0) {
    p->error = "out of mem";
    p->section = "init";
    return 1;
  }
  fill_buffer(size, buf);

  /* send buffer */
  UWORD words = size>>1;
  int res = proto_msg_write_single(proto, test_channel, buf, words);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  /* wait for pending read */
  ULONG got = pamela_wait_event(pb, WAIT_S, WAIT_US, 0);

  /* assume event */
  res = assert_event_mask(p, "wait", pb, got);
  if(res != 0) {
    return res;
  }

  /* assume pending is set again after event */
  if(status->flags != STATUS_FLAGS_PENDING) {
    p->error = "status not pending again";
    p->section = "wait";
    return 1;
  }

  /* expect my channel as pending */
  if(status->pending_channel != test_channel) {
    p->error = "wrong pending channel";
    p->section = "wait";
    return 1;
  }

  /* receive buffer */
  res = proto_msg_read_single(proto, test_channel, buf, &words);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  /* validate buffer */
  res = validate_buffer(size, buf);
  if(res != 0) {
    p->error = "buffer not valid";
    p->section = "post";
    return res;
  }

  FreeVec(buf);

  /* no more pending */
  ULONG got2 = pamela_wait_event(pb, WAIT_S, WAIT_US, 0);

  /* assume event */
  res = assert_timer_mask(p, "post", pb, got2);
  if(res != 0) {
    return res;
  }

  /* assume pending is set again after event */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not none";
    p->section = "post";
    return 1;
  }

  return 0;
}

int test_echo_single(test_t *t, test_param_t *p)
{
  return run_in_test_mode(t, p, run_echo_single, TEST_MODE_ECHO);
}

