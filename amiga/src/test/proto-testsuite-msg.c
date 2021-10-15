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

#include "proto.h"
#include "proto_env.h"
#include "proto_test_shared.h"
#include "proto-testsuite-msg.h"
#include "proto-testsuite.h"
#include "test-buffer.h"

static int msg_write(proto_handle_t *ph, UBYTE chn, UBYTE *buf, UWORD size)
{
  int res = proto_chn_set_tx_size(ph, chn, size);
  if(res != 0) {
    return res;
  }

  return proto_chn_msg_write(ph, chn, buf, size);
}

static int msg_read(proto_handle_t *ph, UBYTE chn, UBYTE *buf, UWORD size)
{
  int res = proto_chn_set_tx_size(ph, chn, size);
  if(res != 0) {
    return res;
  }

  return proto_chn_msg_read(ph, chn, buf, size);
}

// TEST: empty message
int test_msg_empty(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  int res = msg_write(proto, test_channel, NULL, 0);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  res = msg_read(proto, test_channel, NULL, 0);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  return 0;
}

// TEST: tiny message
int test_msg_tiny(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  ULONG data = 0xdeadbeef;

  int res = msg_write(proto, test_channel, (UBYTE *)&data, 2);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  res = msg_read(proto, test_channel, (UBYTE *)&data, 2);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  if (data != 0xdeadbeef)
  {
    p->error = "invalid value";
    p->section = "compare";
    sprintf(p->extra, "%08lx", data);
    return 1;
  }

  return 0;
}

static int msg_read_write(test_t *t, test_param_t *p, ULONG size)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  UBYTE *mem_w = test_buffer_alloc(size, p);
  if (mem_w == NULL)
  {
    return 1;
  }

  ULONG size_r = test_buffer_get_adjusted_size(size, &test_buf_param);
  BYTE *mem_r = test_buffer_alloc(size_r, p);
  if (mem_r == 0)
  {
    test_buffer_free(mem_w);
    return 1;
  }

  test_buffer_fill(size, mem_w, &test_buf_param, p);

  UWORD words = size >> 1;

  /* send buffer */
  int res = msg_write(proto, test_channel, mem_w, words);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  /* receive buffer */
  res = msg_read(proto, test_channel, mem_r, words);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  /* check buf */
  ULONG pos = test_buffer_check(size, mem_w, mem_r);
  if (pos < size)
  {
    p->error = "value mismatch";
    p->section = "compare";
    sprintf(p->extra, "@%08lx: w=%02x r=%02x", pos, (UWORD)mem_w[pos], (UWORD)mem_r[pos]);
    return 1;
  }

  test_buffer_free(mem_w);
  test_buffer_free(mem_r);
  return 0;
}

// TEST: write & read
int test_msg_size(test_t *t, test_param_t *p)
{
  ULONG size = test_buffer_get_default_size(&test_buf_param);
  return msg_read_write(t, p, size);
}

// TEST: write & read max size
int test_msg_size_max(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);
  UWORD max_bytes = 0;

  /* get max words */
  int res = proto_wfunc_read(proto, PROTO_WFUNC_READ_TEST_MAX_WORDS, &max_bytes);
  if(res != PROTO_RET_OK) {
    return res;
  }

  /* MTU is words */
  max_bytes *= 2;

  return msg_read_write(t, p, max_bytes);
}

#if 0
int test_msg_size_chunks(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  ULONG size = test_buffer_get_default_size(&test_buf_param);
  if (size < 4)
  {
    size = 4;
  }

  UBYTE *mem_w = test_buffer_alloc(size, p);
  if (mem_w == 0)
  {
    return 1;
  }

  ULONG size_r = test_buffer_get_adjusted_size(size, &test_buf_param);
  BYTE *mem_r = test_buffer_alloc(size_r, p);
  if (mem_r == 0)
  {
    test_buffer_free(mem_w);
    return 1;
  }

  test_buffer_fill(size, mem_w, &test_buf_param, p);

  ULONG words = size >> 1;
  ULONG c1_words = words >> 1;
  ULONG c2_words = words - c1_words;
  UBYTE *c1_buf = mem_w;
  UBYTE *c2_buf = mem_w + (c1_words << 1);

  /* set tx size */
  int res = proto_chn_set_tx_size(proto, test_channel, words);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "tx_size";
    return res;
  }

  /* send buffer */
  proto_iov_t part2_w = {
      c2_words,
      c2_buf,
      NULL};
  proto_iov_t msgiov_w = {
      c1_words,
      c1_buf,
      &part2_w};
  res = proto_chn_msg_writev(proto, test_channel, &msgiov_w, words);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  ULONG words_r = size_r >> 1;
  c1_buf = mem_r;
  c2_buf = mem_r + (c1_words << 1);
  c2_words = words_r - c1_words;

  /* receive buffer */
  proto_iov_t part2_r = {
      c2_words,
      c2_buf,
      NULL};
  proto_iov_t msgiov_r = {
      c1_words,
      c1_buf,
      &part2_r};
  res = proto_chn_msg_readv(proto, test_channel, &msgiov_r, words_r);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  /* check buf */
  ULONG pos = test_buffer_check(size, mem_w, mem_r);
  if (pos < size)
  {
    p->error = "value mismatch";
    p->section = "compare";
    sprintf(p->extra, "@%08lx: w=%02x r=%02x", pos, (UWORD)mem_w[pos], (UWORD)mem_r[pos]);
    return 1;
  }

  test_buffer_free(mem_w);
  test_buffer_free(mem_r);
  return 0;
}

int test_msg_size_chunks_len(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  ULONG extra = 32;
  ULONG size = test_buffer_get_default_size(&test_buf_param);
  if (size < 4)
  {
    size = 4;
  }

  UBYTE *mem_w = test_buffer_alloc(size + extra, p);
  if (mem_w == 0)
  {
    return 1;
  }

  ULONG size_r = test_buffer_get_adjusted_size(size, &test_buf_param);
  BYTE *mem_r = test_buffer_alloc(size_r, p);
  if (mem_r == 0)
  {
    test_buffer_free(mem_w);
    return 1;
  }

  test_buffer_fill(size, mem_w, &test_buf_param, p);

  ULONG words = size >> 1;
  ULONG c1_words = words >> 1;
  ULONG c2_words = words - c1_words;
  UBYTE *c1_buf = mem_w;
  UBYTE *c2_buf = mem_w + (c1_words << 1);

  /* set tx size */
  int res = proto_chn_set_tx_size(proto, test_channel, words);
  if(res != 0) {
    p->error = proto_perror(res);
    p->section = "tx_size";
    return res;
  }

  /* send buffer */
  proto_iov_t part2_w = {
      c2_words + (extra >> 1), // make buffer longer than total
      c2_buf,
      NULL};
  proto_iov_t msgiov_w = {
      c1_words,
      c1_buf,
      &part2_w};
  res = proto_chn_msg_writev(proto, test_channel, &msgiov_w, words);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  ULONG words_r = size_r >> 1;
  c1_buf = mem_r;
  c2_buf = mem_r + (c1_words << 1);
  c2_words = words_r - c1_words;

  /* receive buffer */
  proto_iov_t part2_r = {
      c2_words + (extra >> 1), // make buffer longer than total
      c2_buf,
      NULL};
  proto_iov_t msgiov_r = {
      c1_words,
      c1_buf,
      &part2_r};
  res = proto_chn_msg_readv(proto, test_channel, &msgiov_r, words_r);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  /* check buf */
  ULONG pos = test_buffer_check(size, mem_w, mem_r);
  if (pos < size)
  {
    p->error = "value mismatch";
    p->section = "compare";
    sprintf(p->extra, "@%08lx: w=%02x r=%02x", pos, (UWORD)mem_w[pos], (UWORD)mem_r[pos]);
    return 1;
  }

  test_buffer_free(mem_w);
  test_buffer_free(mem_r);
  return 0;
}
#endif

int test_msg_write(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);
  ULONG size = test_buffer_get_default_size(&test_buf_param);

  UBYTE *mem_w = test_buffer_alloc(size, p);
  if (mem_w == 0)
  {
    return 1;
  }

  test_buffer_fill(size, mem_w, &test_buf_param, p);

  UWORD words = size >> 1;

  /* send buffer */
  int res = msg_write(proto, test_channel, mem_w, words);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  test_buffer_free(mem_w);
  return 0;
}

int test_msg_write_spi(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);
  ULONG size = test_buffer_get_default_size(&test_buf_param);

  /* set test flags */
  int res = proto_wfunc_write(proto, PROTO_WFUNC_WRITE_TEST_FLAGS, PROTO_TEST_FLAGS_USE_SPI);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "set flags";
    return res;
  }

  UBYTE *mem_w = test_buffer_alloc(size, p);
  if (mem_w == 0)
  {
    return 1;
  }

  test_buffer_fill(size, mem_w, &test_buf_param, p);

  UWORD words = size >> 1;

  /* send buffer */
  res = msg_write(proto, test_channel, mem_w, words);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  test_buffer_free(mem_w);

  /* clear test flags */
  res = proto_wfunc_write(proto, PROTO_WFUNC_WRITE_TEST_FLAGS, 0);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "clear flags";
    return res;
  }

  return 0;
}

int test_msg_write_busy(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);
  ULONG size = test_buffer_get_default_size(&test_buf_param);

  UBYTE *mem_w = test_buffer_alloc(size, p);
  if (mem_w == 0)
  {
    return 1;
  }

  test_buffer_fill(size, mem_w, &test_buf_param, p);

  UWORD words = size >> 1;

  /* enable busy mode */
  int res = proto_action(proto, PROTO_ACTION_TEST_BUSY_LOOP);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "enable busy";
    return 1;
  }

  /* send buffer */
  res = msg_write(proto, test_channel, mem_w, words);
  if (res != PROTO_RET_DEVICE_BUSY)
  {
    p->error = proto_perror(res);
    p->section = "write";
    return 1;
  }

  /* recover from busy mode */
  res = recover_from_busy(proto, p);

  test_buffer_free(mem_w);
  return res;
}

int test_msg_read(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);
  ULONG size = test_buffer_get_default_size(&test_buf_param);
  ULONG size_r = test_buffer_get_adjusted_size(size, &test_buf_param);

  BYTE *mem_r = test_buffer_alloc(size_r, p);
  if (mem_r == 0)
  {
    return 1;
  }

  /* receive buffer */
  UWORD got_words = size_r >> 1;
  int res = msg_read(proto, test_channel, mem_r, got_words);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "read";
    return 1;
  }

  test_buffer_free(mem_r);
  return 0;
}

int test_msg_read_spi(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);
  ULONG size = test_buffer_get_default_size(&test_buf_param);
  ULONG size_r = test_buffer_get_adjusted_size(size, &test_buf_param);

   /* set test flags */
  int res = proto_wfunc_write(proto, PROTO_WFUNC_WRITE_TEST_FLAGS, PROTO_TEST_FLAGS_USE_SPI);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "set flags";
    return res;
  }

  BYTE *mem_r = test_buffer_alloc(size_r, p);
  if (mem_r == 0)
  {
    return 1;
  }

  /* receive buffer */
  UWORD got_words = size_r >> 1;
  res = msg_read(proto, test_channel, mem_r, got_words);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "read";
    return 1;
  }

  test_buffer_free(mem_r);

  /* clear test flags */
  res = proto_wfunc_write(proto, PROTO_WFUNC_WRITE_TEST_FLAGS, 0);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "clear flags";
    return res;
  }

  return 0;
}

int test_msg_read_busy(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);
  ULONG size = test_buffer_get_default_size(&test_buf_param);
  ULONG size_r = test_buffer_get_adjusted_size(size, &test_buf_param);

  BYTE *mem_r = test_buffer_alloc(size_r, p);
  if (mem_r == 0)
  {
    return 1;
  }

  UWORD words = size >> 1;

  /* enable busy mode */
  int res = proto_action(proto, PROTO_ACTION_TEST_BUSY_LOOP);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "enable busy";
    return 1;
  }

  /* receive buffer */
  UWORD got_words = size_r >> 1;
  res = msg_read(proto, test_channel, mem_r, got_words);
  if (res != PROTO_RET_DEVICE_BUSY)
  {
    p->error = proto_perror(res);
    p->section = "read not busy";
    return 1;
  }

  /* recover from busy mode */
  res = recover_from_busy(proto, p);

  test_buffer_free(mem_r);
  return res;
}
