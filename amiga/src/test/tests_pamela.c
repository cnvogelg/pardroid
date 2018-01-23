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

  int res = proto_reset(proto, 1);
  if(res == 0) {
    return 0;
  } else {
    p->error = proto_perror(res);
    p->section = "reset";
    return res;
  }
}

int test_knok_enter_leave(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);

  // make sure knok mode is not enabled
  int res = proto_knok_check(proto);
  if(res == PROTO_KNOK_FOUND) {
    p->error = "KNOK found!";
    p->section = "init";
    return 1;
  }

  // try to enter knock mode
  res = proto_reset(proto, 0);
  if(res != PROTO_RET_OK) {
    p->error = proto_perror(res);
    p->section = "knok enter";
    return res;
  }

  // make sure knok mode is enabled
  res = proto_knok_check(proto);
  if(res == PROTO_KNOK_NOT_FOUND) {
    p->error = "KNOK not found!";
    p->section = "main";
    return 1;
  }

  // try to leave knock mode
  res = proto_knok_exit(proto);
  if(res != PROTO_RET_OK) {
    p->error = proto_perror(res);
    p->section = "knok leave";
    return res;
  }

  // make sure knok mode is not enabled
  res = proto_knok_check(proto);
  if(res == PROTO_KNOK_FOUND) {
    p->error = "KNOK found!";
    p->section = "init";
    return 1;
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

int test_status_timer_sig(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;

  /* init events */
  int res = pamela_init_events(pb);
  if(res != PAMELA_OK) {
    p->error = pamela_perror(res);
    p->section = "pamela_init_events";
    return res;
  }

  /* wait for either timeout or ack */
  ULONG got = pamela_wait_event(pb, 1, 0, 0);

  /* cleanup events */
  pamela_exit_events(pb);

  if((got & pamela_get_timer_sigmask(pb)) == 0) {
    p->error = "no timer triggered";
    p->section = "main";
    return 1;
  }

  if(got & pamela_get_event_sigmask(pb)) {
    p->error = "ack triggered?!";
    p->section = "main";
    return 1;
  }

  return 0;
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
  int res = reg_set(proto, REG_SIM_PENDING, channel);
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
  }

  /* sim_pending with no channel (0xff) */
  res = reg_set(proto, REG_SIM_PENDING, 0xff);
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

int test_status_read_pending_sig(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  status_data_t *status = pamela_get_status(pb);

  /* init events */
  int res = pamela_init_events(pb);
  if(res != PAMELA_OK) {
    p->error = pamela_perror(res);
    p->section = "pamela_init_events";
    return res;
  }

  /* assume no flags set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not init";
    p->section = "pre";
    return 1;
  }

  UWORD channel = (p->iter + test_bias) & 7;

  /* sim pending */
  res = reg_set(proto, REG_SIM_PENDING, channel);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "sim_pending #1";
    return res;
  }

  /* wait for either timeout or event */
  ULONG got = pamela_wait_event(pb, 1, 0, 0);

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
  }

  /* sim pend_req_rem to restore state */
  res = reg_set(proto, REG_SIM_PENDING, 0xff);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "sim_pending #2";
    return res;
  }

  /* wait again and assume return to non-pending state */
  ULONG got2 = pamela_wait_event(pb, 1, 0, 0);

  /* assume pending is cleared again */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not init";
    p->section = "post";
    return 1;
  }

  /* cleanup events */
  pamela_exit_events(pb);

  /* first wait: assume event: pending */
  if(got & pamela_get_timer_sigmask(pb)) {
    p->error = "timer triggered";
    p->section = "main";
    return 1;
  }

  if((got & pamela_get_event_sigmask(pb)) == 0) {
    p->error = "no ack triggered";
    p->section = "main";
    return 1;
  }

  /* second wait: assume event: no pending*/
  if(got2 & pamela_get_timer_sigmask(pb)) {
    p->error = "timer triggered";
    p->section = "main";
    return 1;
  }

  if((got2 & pamela_get_event_sigmask(pb)) == 0) {
    p->error = "no ack triggered";
    p->section = "main";
    return 1;
  }

  return 0;
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
  UBYTE event = (UBYTE)(p->iter + test_bias);
  if(event == 0) {
    event = 1;
  }

  /* simulate an event */
  int res = reg_set(proto, REG_SIM_EVENT, event);
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
  if(status->event_mask != event) {
    p->error = "wrong event mask returned";
    p->section = "main";
    sprintf(p->extra, "got=%02x want=%02x", event, status->event_mask);
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

int test_status_events_sig(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  status_data_t *status = pamela_get_status(pb);

  /* init events */
  int res = pamela_init_events(pb);
  if(res != PAMELA_OK) {
    p->error = pamela_perror(res);
    p->section = "pamela_init_events";
    return res;
  }

  /* assume no flags set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not init";
    p->section = "pre";
    return 1;
  }

  /* create an event */
  UBYTE event = (UBYTE)(p->iter + test_bias);
  if(event == 0) {
    event = 1;
  }

  /* simulate an event */
  res = reg_set(proto, REG_SIM_EVENT, event);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "sim_event #1";
    return res;
  }

  /* wait for either timeout or ack */
  ULONG got = pamela_wait_event(pb, 1, 0, 0);

  /* assume event is set */
  if(status->flags != STATUS_FLAGS_EVENTS) {
    p->error = "status has no events";
    p->section = "main";
    return 1;
  }

  /* check if correct event mask was returned */
  if(status->event_mask != event) {
    p->error = "wrong event mask returned";
    p->section = "main";
    sprintf(p->extra, "got=%02x want=%02x", event, status->event_mask);
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

  /* wait again and assume timeout, no more events */
  ULONG got2 = pamela_wait_event(pb, 1, 0, 0);

  /* assume error is not set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not init";
    p->section = "post";
    return 1;
  }

  /* cleanup events */
  pamela_exit_events(pb);

  /* first wait: assume event */
  if(got & pamela_get_timer_sigmask(pb)) {
    p->error = "timer triggered";
    p->section = "main";
    return 1;
  }

  if((got & pamela_get_event_sigmask(pb)) == 0) {
    p->error = "no ack triggered";
    p->section = "main";
    return 1;
  }

  /* second wait: assume time out */
  if((got2 & pamela_get_timer_sigmask(pb)) == 0) {
    p->error = "no timer triggered";
    p->section = "after";
    return 1;
  }

  if(got2 & pamela_get_event_sigmask(pb)) {
    p->error = "ack triggered";
    p->section = "after";
    return 1;
  }

  return 0;
}

int test_status_attach_detach(test_t *t, test_param_t *p)
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

  /* attach device */
  int res = proto_action(proto, PROTO_ACTION_ATTACH);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "attach failed";
    return res;
  }

  /* update status */
  pamela_update_status(pb);

  /* assume attached flag set */
  if(status->flags != STATUS_FLAGS_ATTACHED) {
    p->error = "status not attached";
    p->section = "main";
    return 1;
  }

  /* detach device */
  res = proto_action(proto, PROTO_ACTION_DETACH);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "detach failed";
    return res;
  }

  /* update status */
  pamela_update_status(pb);

  /* assume error is not set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not init";
    p->section = "post";
    return 1;
  }

  return 0;
}

int test_status_attach_detach_sig(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  status_data_t *status = pamela_get_status(pb);

  /* init events */
  int res = pamela_init_events(pb);
  if(res != PAMELA_OK) {
    p->error = pamela_perror(res);
    p->section = "pamela_init_events";
    return res;
  }

  /* assume no flags set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not init";
    p->section = "pre";
    return 1;
  }

  /* attach device */
  res = proto_action(proto, PROTO_ACTION_ATTACH);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "attach failed";
    return res;
  }

  /* wait for either timeout or ack */
  ULONG got = pamela_wait_event(pb, 1, 0, 0);

  /* assume attached flag set */
  if(status->flags != STATUS_FLAGS_ATTACHED) {
    p->error = "status not attached";
    p->section = "main";
    return 1;
  }

  /* detach device */
  res = proto_action(proto, PROTO_ACTION_DETACH);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "detach failed";
    return res;
  }

  /* wait again and assume event: detach */
  ULONG got2 = pamela_wait_event(pb, 1, 0, 0);

  /* assume error is not set */
  if(status->flags != STATUS_FLAGS_NONE) {
    p->error = "status not init";
    p->section = "post";
    return 1;
  }

  /* cleanup events */
  pamela_exit_events(pb);

  /* first wait: assume event */
  if(got & pamela_get_timer_sigmask(pb)) {
    p->error = "timer triggered";
    p->section = "main";
    return 1;
  }

  if((got & pamela_get_event_sigmask(pb)) == 0) {
    p->error = "no ack triggered";
    p->section = "main";
    return 1;
  }

  /* second wait: assume time out */
  if((got2 & pamela_get_timer_sigmask(pb)) == 0) {
    p->error = "no timer triggered";
    p->section = "after";
    return 1;
  }

  if(got2 & pamela_get_event_sigmask(pb)) {
    p->error = "ack triggered";
    p->section = "after";
    return 1;
  }

  return 0;
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
  if(fw_id != FWID_TEST_PROTO) {
    p->error = "no proto test firmware!";
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
