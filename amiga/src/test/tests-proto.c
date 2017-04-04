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
  UWORD v = (UWORD)p->iter;
  if(params.bias != NULL) {
    v += *params.bias;
  }

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
