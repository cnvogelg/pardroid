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
#include "proto-testsuite-ext.h"
#include "proto-testsuite.h"

int test_chn_set_get_rx_size(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);
  UWORD v = (UWORD)p->iter + test_bias;

  /* set rx size */
  int res = proto_chn_set_rx_size(proto, test_channel, v);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "set rx size";
    return res;
  }

  /* read back */
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

int test_chn_set_get_tx_size(test_t *t, test_param_t *p)
{
  proto_env_handle_t *pb = (proto_env_handle_t *)p->user_data;
  proto_handle_t *proto = proto_env_get_proto(pb);
  UWORD v = (UWORD)p->iter + test_bias;

  /* set tx size */
  int res = proto_chn_set_tx_size(proto, test_channel, v);
  if (res != 0)
  {
    p->error = proto_perror(res);
    p->section = "set rx size";
    return res;
  }

  /* read back */
  UWORD r;
  res = proto_wfunc_read(proto, PROTO_WFUNC_READ_TEST_GET_TX_SIZE, &r);
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
