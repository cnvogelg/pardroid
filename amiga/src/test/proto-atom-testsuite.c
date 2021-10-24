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

#include "timer.h"
#include "pario.h"
#include "proto_atom.h"
#include "proto_env.h"
#include "proto_atom_test_shared.h"
#include "proto-atom-testsuite.h"
#include "test-buffer.h"

// ----- actions -----

int test_action(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  int res = proto_atom_action(proto, TEST_ACTION);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "action";
    return res;
  }

  return 0;
}

int test_action_no_busy(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  int res = proto_atom_action_no_busy(proto, TEST_ACTION);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "action";
    return res;
  }

  return 0;
}

int test_action_bench(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  ULONG delay[2];
  int res = proto_atom_action_bench(proto, TEST_ACTION, delay);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "action";
    return res;
  }

  return 0;
}

int test_busy_action(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  // set busy
  int res = proto_atom_action(proto, TEST_SET_BUSY);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "set busy";
    return res;
  }

  // actions should fail
  res = proto_atom_action(proto, TEST_ACTION);
  if (res != PROTO_RET_DEVICE_BUSY)
  {
    p->error = proto_perror(res);
    p->section = "action not busy";
    return res;
  }

  // this will work
  res = proto_atom_action_no_busy(proto, TEST_ACTION);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "action vs busy";
    return res;
  }

  // clr busy
  res = proto_atom_action_no_busy(proto, TEST_CLR_BUSY);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "clr busy";
    return res;
  }

  return 0;
}

// ----- read/write word -----

int test_read_word(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  UWORD value;
  int res = proto_atom_read_word(proto, TEST_READ_WORD, &value);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "read_word";
    return res;
  }

   if (value != TEST_WORD)
  {
    p->error = "value mismatch";
    p->section = "compare";
    sprintf(p->extra, "w=%04x r=%04x", TEST_WORD, value);
    return 1;
  }

  return 0;
}

int test_write_word(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  UWORD value = 0xbabe;
  int res = proto_atom_write_word(proto, TEST_WRITE_WORD, value);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "write_word";
    return res;
  }

  return 0;
}

int test_busy_word(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  // set busy
  int res = proto_atom_action(proto, TEST_SET_BUSY);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "set busy";
    return res;
  }

  // read word should fail
  UWORD val = 0;
  res = proto_atom_read_word(proto, TEST_READ_WORD, &val);
  if (res != PROTO_RET_DEVICE_BUSY)
  {
    p->error = proto_perror(res);
    p->section = "read word not busy";
    return res;
  }

  // write word should fail
  res = proto_atom_write_word(proto, TEST_WRITE_WORD, val);
  if (res != PROTO_RET_DEVICE_BUSY)
  {
    p->error = proto_perror(res);
    p->section = "write word not busy";
    return res;
  }

  // clr busy
  res = proto_atom_action_no_busy(proto, TEST_CLR_BUSY);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "clr busy";
    return res;
  }

  return 0;
}

// ----- read/write long -----

int test_read_long(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  ULONG value;
  int res = proto_atom_read_long(proto, TEST_READ_LONG, &value);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "read_long";
    return res;
  }

   if (value != TEST_LONG)
  {
    p->error = "value mismatch";
    p->section = "compare";
    sprintf(p->extra, "w=%04lx r=%04lx", TEST_LONG, value);
    return 1;
  }

  return 0;
}

int test_write_long(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  ULONG value = 0xcafebabe;
  int res = proto_atom_write_long(proto, TEST_WRITE_LONG, value);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "write_long";
    return res;
  }

  return 0;
}

int test_busy_lonf(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  // set busy
  int res = proto_atom_action(proto, TEST_SET_BUSY);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "set busy";
    return res;
  }

  // read long should fail
  ULONG val = 0;
  res = proto_atom_read_long(proto, TEST_READ_LONG, &val);
  if (res != PROTO_RET_DEVICE_BUSY)
  {
    p->error = proto_perror(res);
    p->section = "read long not busy";
    return res;
  }

  // write long should fail
  res = proto_atom_write_long(proto, TEST_WRITE_LONG, val);
  if (res != PROTO_RET_DEVICE_BUSY)
  {
    p->error = proto_perror(res);
    p->section = "write long not busy";
    return res;
  }

  // clr busy
  res = proto_atom_action_no_busy(proto, TEST_CLR_BUSY);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "clr busy";
    return res;
  }

  return 0;
}

// ----- read/write block -----

int test_read_block(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  UBYTE *buf = test_buffer_alloc(TEST_BUF_SIZE, p);
  if (buf == NULL)
  {
    return 1;
  }

  int res = proto_atom_read_block(proto, TEST_READ_BLOCK, buf, TEST_BUF_SIZE);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "read_block";
    return res;
  }

  int errors = 0;
  for(int i=0;i<TEST_BUF_SIZE;i++) {
    UBYTE val = (UBYTE)((i + TEST_BYTE_OFFSET) & 0xff);
    if (buf[i] != val)
    {
      p->error = "value mismatch";
      p->section = "compare";
      sprintf(p->extra, "@%ld: w=%04lx r=%04lx (errors=%d)", (LONG)i, (ULONG)val, (ULONG)buf[i], errors);
      errors ++;
    }
  }

  test_buffer_free(buf);

  if(errors > 0) {
    return 1;
  }
  return 0;
}

int test_write_block(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  UBYTE *buf = test_buffer_alloc(TEST_BUF_SIZE, p);
  if (buf == NULL)
  {
    return 1;
  }

  for(int i=0;i<TEST_BUF_SIZE;i++) {
    buf[i] = (UBYTE)((i + TEST_BYTE_OFFSET) & 0xff);
  }

  int res = proto_atom_write_block(proto, TEST_WRITE_BLOCK, buf, TEST_BUF_SIZE);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "write_block";
    return res;
  }

  test_buffer_free(buf);

  return 0;
}

int test_busy_block(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  UBYTE *buf = test_buffer_alloc(TEST_BUF_SIZE, p);
  if (buf == NULL)
  {
    return 1;
  }

  // set busy
  int res = proto_atom_action(proto, TEST_SET_BUSY);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "set busy";
    return res;
  }

  // read block should fail
  res = proto_atom_read_block(proto, TEST_READ_BLOCK, buf, TEST_BUF_SIZE);
  if (res != PROTO_RET_DEVICE_BUSY)
  {
    p->error = proto_perror(res);
    p->section = "read block not busy";
    return res;
  }

  // write block should fail
  res = proto_atom_write_block(proto, TEST_WRITE_BLOCK, buf, TEST_BUF_SIZE);
  if (res != PROTO_RET_DEVICE_BUSY)
  {
    p->error = proto_perror(res);
    p->section = "write block not busy";
    return res;
  }

  // clr busy
  res = proto_atom_action_no_busy(proto, TEST_CLR_BUSY);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "clr busy";
    return res;
  }

  test_buffer_free(buf);

  return 0;
}

