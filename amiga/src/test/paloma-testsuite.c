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

#include "test.h"
#include "test-buffer.h"

#include "paloma_lib.h"
#include "paloma-testsuite.h"
#include "test/paloma.h"

#define CHECK_PALOMA_RES(res, sec) \
  if (res != 0) \
  { \
    p->error = pamela_perror(res); \
    p->section = sec; \
    return res; \
  }

TEST_FUNC(test_init_exit)
{
  // nothing to do. init/exit is done in all tests.
  return 0;
}

TEST_FUNC(test_load_save_reset)
{
  paloma_handle_t *ph = (paloma_handle_t *)p->user_data;
  int res = 0;

  res = paloma_param_all_save(ph);
  CHECK_PALOMA_RES(res, "save");

  res = paloma_param_all_reset(ph);
  CHECK_PALOMA_RES(res, "reset");

  res = paloma_param_all_load(ph);
  CHECK_PALOMA_RES(res, "load");

  return 0;
}
