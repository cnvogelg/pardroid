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

#include "paloma.h"

#include "fwid.h"
#include "test.h"

int test_paloma_init_exit(test_t *t, test_param_t *p)
{
  /* do nothing */
  return 0;
}

int test_handler_open_close(test_t *t, test_param_t *p)
{
  paloma_handle_t *pal = (paloma_handle_t *)p->user_data;
  return 0;
}
