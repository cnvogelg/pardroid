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

#include "fwid.h"
#include "test.h"
#include "proto-testsuite.h"
#include "proto_test_shared.h"

test_buffer_param_t test_buf_param;
UBYTE test_channel;

void tests_proto_config(UWORD size, UWORD bias, UWORD add_size, UWORD sub_size,
                        UBYTE channel)
{
  test_buf_param.size = size;
  test_buf_param.bias = bias;
  test_buf_param.add_size = add_size;
  test_buf_param.sub_size = sub_size;
  test_channel = channel;
}

// ----- helper -----

int recover_from_busy(proto_handle_t *proto, test_param_t *p)
{
  int res = 0;

  for (int i = 0; i < 10; i++)
  {
    res = proto_ping(proto);
    if (res == PROTO_RET_OK)
    {
      return 0;
    }
    else if (res != PROTO_RET_DEVICE_BUSY)
    {
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
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  /* perform reset */
  int res = proto_reset(proto);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "reset";
    return res;
  }

  return 0;
}

int test_knok(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  /* enter knok */
  int res = proto_knok(proto);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "knok";
    return res;
  }

  /* ping action must fail in knok mode */
  res = proto_ping(proto);
  if (res != PROTO_RET_DEVICE_BUSY)
  {
    p->error = proto_perror(res);
    p->section = "ping must fail with timeout";
    return res;
  }

  /* perform reset */
  res = proto_reset(proto);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "reset";
    return res;
  }

  return 0;
}

int test_ping(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  int res = proto_ping(proto);
  if (res == 0)
  {
    return 0;
  }
  else
  {
    p->error = proto_perror(res);
    p->section = "ping";
    return res;
  }
}

int test_ping_busy(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  /* enable busy mode */
  int res = proto_action(proto, PROTO_ACTION_TEST_BUSY_LOOP);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "enable busy";
    return 1;
  }

  /* ping must be busy */
  res = proto_ping(proto);
  if (res != PROTO_RET_DEVICE_BUSY)
  {
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
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);
  UWORD v = 0xbabe + (UWORD)p->iter + test_buf_param.bias;

  /* write */
  int res = proto_wfunc_write(proto, PROTO_WFUNC_WRITE_TEST_VALUE, v);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  /* read back */
  UWORD r;
  res = proto_wfunc_read(proto, PROTO_WFUNC_READ_TEST_VALUE, &r);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  /* check */
  if (v != r)
  {
    p->error = "value mismatch";
    p->section = "compare";
    sprintf(p->extra, "w=%04x r=%04x", v, r);
    return 1;
  }

  return 0;
}

int test_wfunc_busy(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);
  UWORD v = 0xbabe + (UWORD)p->iter + test_buf_param.bias;

  /* enable busy mode */
  int res = proto_action(proto, PROTO_ACTION_TEST_BUSY_LOOP);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "enable busy";
    return 1;
  }

  /* write */
  res = proto_wfunc_write(proto, PROTO_WFUNC_WRITE_TEST_VALUE, v);
  if (res != PROTO_RET_DEVICE_BUSY)
  {
    p->error = proto_perror(res);
    p->section = "write not busy";
    return res;
  }

  /* read back */
  UWORD r;
  res = proto_wfunc_read(proto, PROTO_WFUNC_READ_TEST_VALUE, &r);
  if (res != PROTO_RET_DEVICE_BUSY)
  {
    p->error = proto_perror(res);
    p->section = "read not busy";
    return res;
  }

  return recover_from_busy(proto, p);
}

int test_lfunc_write_read(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);
  UWORD val = (UWORD)p->iter + test_buf_param.bias;
  ULONG v = 0xdeadbeef + val;

  /* write */
  int res = proto_lfunc_write(proto, PROTO_LFUNC_WRITE_TEST_VALUE, v);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "write";
    return res;
  }

  /* read back */
  ULONG r;
  res = proto_lfunc_read(proto, PROTO_LFUNC_WRITE_TEST_VALUE, &r);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "read";
    return res;
  }

  /* check */
  if (v != r)
  {
    p->error = "value mismatch";
    p->section = "compare";
    sprintf(p->extra, "w=%04lx r=%04lx", v, r);
    return 1;
  }

  return 0;
}

int test_lfunc_busy(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);
  UWORD val = (UWORD)p->iter + test_buf_param.bias;
  ULONG v = 0xdeadbeef + val;

  /* enable busy mode */
  int res = proto_action(proto, PROTO_ACTION_TEST_BUSY_LOOP);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "enable busy";
    return 1;
  }

  /* write */
  res = proto_lfunc_write(proto, PROTO_LFUNC_WRITE_TEST_VALUE, v);
  if (res != PROTO_RET_DEVICE_BUSY)
  {
    p->error = proto_perror(res);
    p->section = "write not busy";
    return res;
  }

  /* read back */
  ULONG r;
  res = proto_lfunc_read(proto, PROTO_LFUNC_READ_TEST_VALUE, &r);
  if (res != PROTO_RET_DEVICE_BUSY)
  {
    p->error = proto_perror(res);
    p->section = "read not busy";
    return res;
  }

  return recover_from_busy(proto, p);
}
