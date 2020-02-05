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
#include "proto-testsuite-ext.h"
#include "proto-testsuite.h"

int test_chn_set_get_size(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);
  UWORD v = (UWORD)p->iter + test_buf_param.bias;

  /* set tx size */
  int res = proto_chn_set_tx_size(proto, test_channel, v);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "set rx size";
    return res;
  }

  /* get rx size */
  UWORD r;
  res = proto_chn_get_rx_size(proto, test_channel, &r);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "get rx size";
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

int test_chn_set_get_offset(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);
  ULONG v = p->iter + test_buf_param.bias + 0xdeadbeef;

  /* set offset */
  int res = proto_chn_set_offset(proto, test_channel, v);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "set rx offset";
    return res;
  }

  /* read back */
  ULONG r;
  res = proto_lfunc_read(proto, PROTO_LFUNC_READ_TEST_OFFSET, &r);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "get rx offset";
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

int test_chn_cancel_transfer(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);

  /* clear test flags */
  int res = proto_wfunc_write(proto, PROTO_WFUNC_WRITE_TEST_FLAGS, 0);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "clear flags";
    return res;
  }

  /* trigger cancel */
  ULONG r;
  res = proto_chn_cancel_transfer(proto, test_channel);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "cancel rx";
    return res;
  }

  /* read flags */
  UWORD flags;
  res = proto_wfunc_read(proto, PROTO_WFUNC_READ_TEST_FLAGS, &flags);
  if(res != 0)
  {
    p->error = proto_perror(res);
    p->section = "read flags";
    return res;
  }

  /* check flags */
  if (flags != PROTO_TEST_FLAGS_CANCEL_TRANSFER)
  {
    p->error = "no rx cancel flag";
    p->section = "compare";
    sprintf(p->extra, "flags=%04x", flags);
    return 1;
  }

  return 0;
}
