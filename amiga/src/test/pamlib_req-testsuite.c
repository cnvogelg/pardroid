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

#include "pamlib_req.h"
#include "pamlib_req-testsuite.h"

#include "test/pamela.h"

#define TEST_DEFAULT_
#define TEST_BYTE_OFFSET 3
#define TEST_SEEK       0xdeadbeefUL

#define CHECK_PAMLIB_RES(res, sec) \
  if (res != 0) \
  { \
    p->error = pamela_perror(res); \
    p->section = sec; \
    return res; \
  }

#define CHECK_PAMLIB_RES_VAL(res, sec, val) \
  if (res != val) \
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

TEST_FUNC(test_open_close)
{
  pamlib_handle_t *ph = (pamlib_handle_t *)p->user_data;
  int res = 0;

  // open req channel
  pamlib_req_t *req = pamlib_req_open(ph, p->port, &res);
  CHECK_PAMLIB_RES(res, "open");

  // close req channel
  res = pamlib_req_close(req);
  CHECK_PAMLIB_RES(res, "close");

  return 0;
}

static int test_transfer_helper(test_param_t *p, UWORD req_size)
{
  pamlib_handle_t *ph = (pamlib_handle_t *)p->user_data;
  int res = 0;

  UBYTE *buf = test_buffer_alloc(TEST_BUF_SIZE, p);
  if (buf == NULL)
  {
    return 1;
  }

  // fill buffer
  for(int i=0;i<req_size;i++) {
    UBYTE val = (UBYTE)((i + TEST_BYTE_OFFSET) & 0xff);
    buf[i] = val;
  }

  // open req
  pamlib_req_t *req = pamlib_req_open(ph, p->port, &res);
  CHECK_PAMLIB_RES(res, "open");

  // transfer req
  res = pamlib_req_transfer(req, buf, req_size);
  CHECK_PAMLIB_RES_VAL(res, "transfer", req_size);

  // close channel
  res = pamlib_req_close(req);
  CHECK_PAMLIB_RES(res, "close");

  // check buffer
  int errors = 0;
  for(int i=0;i<req_size;i++) {
    UBYTE val = (UBYTE)((i + TEST_BYTE_OFFSET) & 0xff);
    val++;
    if (buf[i] != val)
    {
      p->error = "value mismatch";
      p->section = "compare";
      sprintf(p->extra, "@%ld: w=%04lx r=%04lx (errors=%d)", (LONG)i, (ULONG)val, (ULONG)buf[i], errors);
      errors ++;
    }
  }

  test_buffer_free(buf);

  return 0;
}

TEST_FUNC(test_transfer)
{
  return test_transfer_helper(p, TEST_BUF_SIZE);
}

TEST_FUNC(test_transfer_odd)
{
  return test_transfer_helper(p, TEST_BUF_SIZE - 1);
}
