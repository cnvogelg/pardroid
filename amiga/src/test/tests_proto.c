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
#include "test.h"

#define REG_TEST 0

static UWORD test_size;
static UWORD test_bias;
static UWORD test_add_size;
static UWORD test_sub_size;

void tests_proto_config(UWORD size, UWORD bias, UWORD add_size, UWORD sub_size)
{
  test_size = size;
  test_bias = bias;
  test_add_size = add_size;
  test_sub_size = sub_size;
}

int test_ping(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;
  int res = proto_cmd(pb->proto, PROTO_CMD_PING);
  if(res == 0) {
    return 0;
  } else {
    p->error = proto_perror(res);
    p->section = "ping";
    return res;
  }
}

int test_reset(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;
  int res = proto_cmd(pb->proto, PROTO_CMD_RESET);
  if(res == 0) {
    return 0;
  } else {
    p->error = proto_perror(res);
    p->section = "reset";
    return res;
  }
}

int test_reg_write(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;
  UWORD v = 0x4711;

  int res = proto_reg_rw_write(pb->proto, REG_TEST, &v);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }
  return 0;
}

int test_reg_read(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;
  UWORD v;

  int res = proto_reg_rw_read(pb->proto, REG_TEST, &v);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }
  return 0;
}

int test_reg_write_read(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;
  UWORD v = (UWORD)p->iter + test_bias;

  /* write */
  int res = proto_reg_rw_write(pb->proto, REG_TEST, &v);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  /* read back */
  UWORD r;
  res = proto_reg_rw_read(pb->proto, REG_TEST, &r);
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

int test_msg_empty(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;
  int res = proto_msg_write_single(pb->proto, 0, 0, 0);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  ULONG size = 0;
  res = proto_msg_read_single(pb->proto, 0, 0, &size);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  if(size != 0) {
    p->error = "not empty";
    p->section = "compare";
    sprintf(p->extra, "%04lx", size);
    return 1;
  }

  return 0;
}

int test_msg_tiny(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;
  ULONG data = 0xdeadbeef;
  int res = proto_msg_write_single(pb->proto, 0, (UBYTE *)&data, 2);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  ULONG size = 2;
  res = proto_msg_read_single(pb->proto, 0, (UBYTE *)&data, &size);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  if(size != 2) {
    p->error = "not two words";
    p->section = "compare";
    sprintf(p->extra, "%04lx", size);
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

int test_msg_size(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;
  ULONG size = get_default_size();

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

  /* send buffer */
  int res = proto_msg_write_single(pb->proto, 0, mem_w, words);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  /* receive buffer */
  ULONG got_words = size_r>>1;
  res = proto_msg_read_single(pb->proto, 0, mem_r, &got_words);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  /* check buf size */
  if(got_words != words) {
    p->error = "size mismatch";
    p->section = "compare";
    sprintf(p->extra, "w=%04lx r=%04lx", words, got_words);
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

int test_msg_size_chunks(test_t *t, test_param_t *p)
{
  parbox_handle_t *pb = (parbox_handle_t *)p->user_data;
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
  ULONG msgiov_w[] = {
    words,
    c1_words,
    (ULONG)c1_buf,
    c2_words,
    (ULONG)c2_buf,
    0
  };
  int res = proto_msg_write(pb->proto, 0, msgiov_w);
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
  ULONG msgiov_r[] = {
    words_r,
    c1_words,
    (ULONG)c1_buf,
    c2_words,
    (ULONG)c2_buf,
    0
  };
  res = proto_msg_read(pb->proto, 0, msgiov_r);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  /* check buf size */
  if(msgiov_r[0] != words) {
    p->error = "size mismatch";
    p->section = "compare";
    sprintf(p->extra, "w=%04lx r=%04lx", words, msgiov_r[0]);
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
