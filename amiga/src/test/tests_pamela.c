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

#define ACTION_TRIGGER_SIGNAL   15
#define ACTION_BUSY_LOOP        14

void tests_proto_config(UWORD size, UWORD bias, UWORD add_size, UWORD sub_size,
                        UBYTE channel)
{
  test_size = size;
  test_bias = bias;
  test_add_size = add_size;
  test_sub_size = sub_size;
  test_channel = channel;
}

// ----- helper -----

static int recover_from_busy(proto_handle_t *proto, test_param_t *p)
{
  int res = 0;

  for(int i=0;i<10;i++) {
    res = proto_ping(proto);
    if(res == PROTO_RET_OK) {
      return 0;
    }
    else if(res != PROTO_RET_DEVICE_BUSY) {
      p->error = proto_perror(res);
      p->section = "recover loop";
      return 1;
    }
    Delay(10);
  }
  p->error = proto_perror(res);
  p->section = "recover end";
  return 1;
}

// ----- actions -----

int test_reset(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);

  /* perform reset */
  int res = proto_reset(proto);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "reset";
    return res;
  }

  return 0;
}

int test_knok(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);

  /* enter knok */
  int res = proto_knok(proto);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "knok";
    return res;
  }

  /* ping action must fail in knok mode */
  res = proto_ping(proto);
  if(res != PROTO_RET_DEVICE_BUSY) {
    p->error = proto_perror(res);
    p->section = "ping must fail with timeout";
    return res;
  }

  /* perform reset */
  res = proto_reset(proto);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "reset";
    return res;
  }

  return 0;
}

int test_ping(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);

  int res = proto_ping(proto);
  if(res == 0) {
    return 0;
  } else {
    p->error = proto_perror(res);
    p->section = "ping";
    return res;
  }
}

int test_ping_busy(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);

  /* enable busy mode */
  int res = proto_action(proto, ACTION_BUSY_LOOP);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "enable busy";
    return 1;
  }

  /* ping must be busy */
  res = proto_ping(proto);
  if(res != PROTO_RET_DEVICE_BUSY) {
    p->error = proto_perror(res);
    p->section = "ping not busy";
    return res;
  }

  /* recover already does pings */
  return recover_from_busy(proto, p);
}

// ----- functions -----

int test_wfunc_write_read(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  UWORD v = 0xbabe + (UWORD)p->iter + test_bias;

  /* write */
  int res = proto_function_write_word(proto, PROTO_WFUNC_USER, v);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  /* read back */
  UWORD r;
  res = proto_function_read_word(proto, PROTO_WFUNC_USER, &r);
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

int test_wfunc_busy(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  UWORD v = 0xbabe + (UWORD)p->iter + test_bias;

  /* enable busy mode */
  int res = proto_action(proto, ACTION_BUSY_LOOP);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "enable busy";
    return 1;
  }

  /* write */
  res = proto_function_write_word(proto, PROTO_WFUNC_USER, v);
  if(res != PROTO_RET_DEVICE_BUSY) {
    p->error = proto_perror(res);
    p->section = "write not busy";
    return res;
  }

  /* read back */
  UWORD r;
  res = proto_function_read_word(proto, PROTO_WFUNC_USER, &r);
  if(res != PROTO_RET_DEVICE_BUSY) {
    p->error = proto_perror(res);
    p->section = "read not busy";
    return res;
  }

  return recover_from_busy(proto, p);
}

int test_lfunc_write_read(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  UWORD val = (UWORD)p->iter + test_bias;
  ULONG v = 0xdeadbeef + val;

  /* write */
  int res = proto_function_write_long(proto, PROTO_WFUNC_USER, v);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  /* read back */
  ULONG r;
  res = proto_function_read_long(proto, PROTO_WFUNC_USER, &r);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  /* check */
  if(v != r) {
    p->error = "value mismatch";
    p->section = "compare";
    sprintf(p->extra, "w=%04lx r=%04lx", v, r);
    return 1;
  }

  return 0;
}

int test_lfunc_busy(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);
  UWORD val = (UWORD)p->iter + test_bias;
  ULONG v = 0xdeadbeef + val;

  /* enable busy mode */
  int res = proto_action(proto, ACTION_BUSY_LOOP);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "enable busy";
    return 1;
  }

  /* write */
  res = proto_function_write_long(proto, PROTO_WFUNC_USER, v);
  if(res != PROTO_RET_DEVICE_BUSY) {
    p->error = proto_perror(res);
    p->section = "write not busy";
    return res;
  }

  /* read back */
  ULONG r;
  res = proto_function_read_long(proto, PROTO_WFUNC_USER, &r);
  if(res != PROTO_RET_DEVICE_BUSY) {
    p->error = proto_perror(res);
    p->section = "read not busy";
    return res;
  }

  return recover_from_busy(proto, p);
}

// ----- messages -----

int test_msg_empty(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);

  int res = proto_msg_write_single(proto, test_channel, 0, 0, 0);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  UWORD size = 0;
  UWORD crc = 0;
  res = proto_msg_read_single(proto, test_channel, 0, &size, &crc);
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
  UWORD crc = 0xcafe;

  int res = proto_msg_write_single(proto, test_channel, (UBYTE *)&data, 2, crc);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  UWORD size = 2;
  UWORD got_crc = 0;
  res = proto_msg_read_single(proto, test_channel, (UBYTE *)&data, &size, &got_crc);
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

  if(crc != got_crc) {
    p->error = "invalid crc";
    p->section = "compare";
    sprintf(p->extra, "%04x != %04x", crc, got_crc);
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
  UWORD crc = 0xcafe;

  /* send buffer */
  int res = proto_msg_write_single(proto, test_channel, mem_w, words, crc);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  /* receive buffer */
  UWORD got_words = size_r>>1;
  UWORD got_crc = 0;
  res = proto_msg_read_single(proto, test_channel, mem_r, &got_words, &got_crc);
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

  /* check crc */
  if(crc != got_crc) {
    p->error = "invalid crc";
    p->section = "compare";
    sprintf(p->extra, "%04x != %04x", crc, got_crc);
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
  int res = proto_function_read_word(proto, PROTO_WFUNC_USER+1, &max_bytes);
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

  /* check crc */
  if(msgiov_w.crc != msgiov_r.crc) {
    p->error = "crc mismatch";
    p->section = "compare";
    sprintf(p->extra, "w=%04x r=%04x", msgiov_w.crc, msgiov_r.crc);
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
  UWORD crc = 0x4711;

  /* send buffer */
  int res = proto_msg_write_single(proto, test_channel, mem_w, words, crc);
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
  int res = proto_function_read_word(proto, PROTO_WFUNC_USER+1, &size);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read max_bytes";
    return 1;
  }

  /* too many now */
  size += 2;

  UBYTE *mem_w = AllocVec(size, MEMF_PUBLIC);
  if(mem_w == 0) {
    p->error = "out of mem";
    p->section = "init";
    return 1;
  }

  mem_w[0] = 0xaa;
  mem_w[1] = 0x55;

  fill_buffer(size, mem_w);

  UWORD words = size>>1;
  UWORD crc = 0x1234;

  /* send buffer and expect msg to large */
  res = proto_msg_write_single(proto, test_channel, mem_w, words, crc);
  if(res != PROTO_RET_MSG_TOO_LARGE) {
    p->error = proto_perror(res);
    p->section = "write must return MSG_TOO_LARGE";
    return 1;
  }

  /* we have to reset now */
  res = proto_reset(proto);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "reset";
    return 1;
  }

  FreeVec(mem_w);
  return 0;
}

int test_msg_write_busy(test_t *t, test_param_t *p)
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
  UWORD crc = 0x4711;

  /* enable busy mode */
  int res = proto_action(proto, ACTION_BUSY_LOOP);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "enable busy";
    return 1;
  }

  /* send buffer */
  res = proto_msg_write_single(proto, test_channel, mem_w, words, crc);
  if(res != PROTO_RET_DEVICE_BUSY) {
    p->error = proto_perror(res);
    p->section = "write";
    return 1;
  }

  /* recover from busy mode */
  res = recover_from_busy(proto, p);

  FreeVec(mem_w);
  return res;
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
  UWORD got_crc = 0;
  int res = proto_msg_read_single(proto, test_channel, mem_r, &got_words, &got_crc);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read";
    return 1;
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
  int res = proto_function_read_word(proto, PROTO_WFUNC_USER+1, &size);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read max_bytes";
    return 1;
  }

  /* too many now */
  size += 2;

  /* write too large size */
  res = proto_function_write_word(proto, PROTO_WFUNC_USER+2, size);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write too large size";
    return 1;
  }

  BYTE *mem_r = AllocVec(size, MEMF_PUBLIC);
  if(mem_r == 0) {
    p->error = "out of mem";
    p->section = "init";
    return 1;
  }

  UWORD words = size>>1;
  UWORD crc = 0;

  /* receive buffer */
  res = proto_msg_read_single(proto, test_channel, mem_r, &words, &crc);
  if(res != PROTO_RET_MSG_TOO_LARGE) {
    p->error = proto_perror(res);
    p->section = "read must return MSG_TOO_LARGE";
    return 1;
  }

  /* we have to reset now */
  res = proto_reset(proto);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "reset";
    return 1;
  }

  FreeVec(mem_r);
  return 0;
}

int test_msg_read_busy(test_t *t, test_param_t *p)
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

  /* enable busy mode */
  int res = proto_action(proto, ACTION_BUSY_LOOP);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "enable busy";
    return 1;
  }

  /* receive buffer */
  UWORD got_words = size_r>>1;
  UWORD got_crc = 0;
  res = proto_msg_read_single(proto, test_channel, mem_r, &got_words, &got_crc);
  if(res != PROTO_RET_DEVICE_BUSY) {
    p->error = proto_perror(res);
    p->section = "read not busy";
    return 1;
  }

  /* recover from busy mode */
  res = recover_from_busy(proto, p);

  FreeVec(mem_r);
  return res;
}

/* ---------- status tests ----------------------------------------------- */

#define WAIT_S      0UL
#define WAIT_US     50000UL

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

static int assert_trigger_mask(test_param_t *p, const char *section,
                               pamela_handle_t *pb, ULONG got)
{
  ULONG trigger_mask = pamela_get_trigger_sigmask(pb);

  if(got != trigger_mask) {
    p->error = "no trigger found";
    p->section = section;
    sprintf(p->extra, "got=%08lx want=%08lx", got, trigger_mask);
    return 1;
  }

  return 0;
}

static int assert_num_triggers(test_param_t *p, const char *section,
                               pamela_handle_t *pb,
                               UWORD num_triggers, UWORD num_signals)
{
  UWORD got_triggers = pamela_get_num_triggers(pb);
  UWORD got_signals = pamela_get_num_trigger_signals(pb);

  /* check number of trigger aka irqs */
  if(num_triggers != got_triggers) {
    p->error = "num triggers (ack irqs) mismatch";
    p->section = section;
    sprintf(p->extra, "got=%u want=%u", got_triggers, num_triggers);
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
    return 1;
  }

  /* call test func */
  res = func(t, p);

  /* cleanup events */
  pamela_exit_events(pb);

  return res;
}

static int run_timer_sig(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;

  /* wait for either timeout or ack */
  ULONG got = pamela_wait_event(pb, WAIT_S, WAIT_US, 0);

  int res = assert_timer_mask(p, "main", pb, got);
  if(res != 0) {
    return 1;
  }

  return assert_num_triggers(p, "main", pb, 0, 0);
}

int test_timer_sig(test_t *t, test_param_t *p)
{
  return run_with_events(t, p, run_timer_sig);
}

static int run_event_sig(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);

  /* trigger signal */
  int res = proto_action(proto, ACTION_TRIGGER_SIGNAL);
  if(res != 0) {
    p->error = pamela_perror(res);
    p->section = "action to trigger signal";
    return 1;
  }

  /* wait for either timeout or trigger signal */
  ULONG got = pamela_wait_event(pb, WAIT_S, WAIT_US, 0);

  res = assert_trigger_mask(p, "main", pb, got);
  if(res != 0) {
    return 1;
  }

  return assert_num_triggers(p, "main", pb, 1, 1);
}

int test_event_sig(test_t *t, test_param_t *p)
{
  return run_with_events(t, p, run_event_sig);
}

static int run_event_sig2(test_t *t, test_param_t *p)
{
  pamela_handle_t *pb = (pamela_handle_t *)p->user_data;
  proto_handle_t *proto = pamela_get_proto(pb);

  /* trigger signal */
  int res = proto_action(proto, ACTION_TRIGGER_SIGNAL);
  if(res != 0) {
    p->error = pamela_perror(res);
    p->section = "action to trigger signal";
    return 1;
  }

  /* trigger signal 2 */
  res = proto_action(proto, ACTION_TRIGGER_SIGNAL);
  if(res != 0) {
    p->error = pamela_perror(res);
    p->section = "action to trigger signal 2";
    return 1;
  }

  /* wait for either timeout or trigger signal */
  ULONG got = pamela_wait_event(pb, WAIT_S, WAIT_US, 0);

  res = assert_trigger_mask(p, "main", pb, got);
  if(res != 0) {
    return 1;
  }

  return assert_num_triggers(p, "main", pb, 2, 1);
}

int test_event_sig2(test_t *t, test_param_t *p)
{
  return run_with_events(t, p, run_event_sig2);
}
