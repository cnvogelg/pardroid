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
#include "proto_dev.h"
#include "proto-dev-testsuite.h"
#include "test-buffer.h"
#include "fwid.h"

// ----- actions -----

int test_ping(test_t *t, test_param_t *p)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;

  int res = proto_dev_action_ping(proto);
  if (res != 0)
  {
    p->error = proto_atom_perror(res);
    p->section = "ping";
    return res;
  }

  return 0;
}

int test_fwid(test_t *t, test_param_t *p)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;

  // fw id
  UWORD fw_id;
  int res = proto_dev_get_fw_id(proto, &fw_id);
  if (res != 0)
  {
    p->error = proto_atom_perror(res);
    p->section = "get_fw_id";
    return res;
  }

  // check id
  if(fw_id != FWID_TEST_PROTO_DEV) {
    p->error = "wrong fw_id";
    p->section = "get_fw_id";
    sprintf(p->extra, "got=%04x want=%04x", fw_id, FWID_TEST_PROTO_DEV);
  }

  return 0;
}

int test_fwver(test_t *t, test_param_t *p)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;

  // fw ver
  UWORD fw_ver;
  int res = proto_dev_get_fw_version(proto, &fw_ver);
  if (res != 0)
  {
    p->error = proto_atom_perror(res);
    p->section = "get_fw_version";
    return res;
  }

  return 0;
}

int test_machtag(test_t *t, test_param_t *p)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;

  // machtag
  UWORD machtag;
  int res = proto_dev_get_mach_tag(proto, &machtag);
  if (res != 0)
  {
    p->error = proto_atom_perror(res);
    p->section = "get_machtag";
    return res;
  }

  return 0;
}

int test_drvtok(test_t *t, test_param_t *p)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;

  // set token
  UWORD token = 0xbabe;
  int res = proto_dev_set_driver_token(proto, token);
  if (res != 0)
  {
    p->error = proto_atom_perror(res);
    p->section = "set token";
    return res;
  }

  // get token
  UWORD got_token = 0;
  res = proto_dev_get_driver_token(proto, &got_token);
  if (res != 0)
  {
    p->error = proto_atom_perror(res);
    p->section = "get token";
    return res;
  }

  // check token
  if(token != got_token) {
    p->error = "token mismatch";
    p->section = "check";
    sprintf(p->extra, "got=%04x want=%04x", got_token, token);
  }

  return 0;
}
