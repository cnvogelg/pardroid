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

// ----- actions -----

int test_reset(test_t *t, test_param_t *p)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;

  int res = proto_dev_action_reset(proto);
  if (res != 0)
  {
    p->error = proto_atom_perror(res);
    p->section = "reset";
    return res;
  }

  return 0;
}
