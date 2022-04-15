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

#include "pamela.h"
#include "pamela-testsuite.h"

#define CHECK_PAM_RES(res, sec) \
  if (res != 0) \
  { \
    p->error = pamela_perror(res); \
    p->section = sec; \
    return res; \
  }

#define CHECK_PAM_RES_VAL(res, sec, val) \
  if (res != val) \
  { \
    p->error = pamela_perror(res); \
    p->section = sec; \
    return res; \
  }

#define TEST_PORT  1234
#define TEST_BUF_SIZE  512
#define TEST_BYTE_OFFSET 3

TEST_FUNC(test_init_exit)
{
  // nothing to do. init/exit is done in all tests.
  return 0;
}

TEST_FUNC(test_open_close)
{
  pamela_handle_t *pam = (pamela_handle_t *)p->user_data;
  pamela_channel_t *chn = NULL;
  int res = 0;

  chn = pamela_open(pam, TEST_PORT, &res);
  CHECK_PAM_RES(res, "open");
  CHECK_NOT_NULL(chn, "open");

  res = pamela_close(chn);
  CHECK_PAM_RES(res, "close");

  return 0;
}

TEST_FUNC(test_read)
{
  pamela_handle_t *pam = (pamela_handle_t *)p->user_data;
  pamela_channel_t *chn = NULL;
  int res = 0;

  UBYTE *buf = test_buffer_alloc(TEST_BUF_SIZE, p);
  if (buf == NULL)
  {
    return 1;
  }

  // open channel
  chn = pamela_open(pam, TEST_PORT, &res);
  CHECK_PAM_RES(res, "open");
  CHECK_NOT_NULL(chn, "open");

  // set mtu
  res = pamela_set_mtu(chn, TEST_BUF_SIZE);
  CHECK_PAM_RES(res, "set_mtu");
  UWORD mtu = 0;
  res = pamela_get_mtu(chn, &mtu);
  CHECK_EQUAL(mtu, TEST_BUF_SIZE, "mtu mismatch");

  // post read request
  res = pamela_read_request(chn, TEST_BUF_SIZE);
  CHECK_PAM_RES(res, "read_req");

  // wait for event
  res = pamela_event_wait(pam, 1, 0, NULL);
  CHECK_EQUAL(res, PAMELA_WAIT_EVENT, "wait: no event");

  // update state
  res = pamela_event_update(pam, NULL);
  CHECK_PAM_RES(res, "event_update");

  // check status
  UWORD status = pamela_status(chn);
  UWORD exp = PAMELA_STATUS_ACTIVE | PAMELA_STATUS_READ_READY;
  CHECK_EQUAL(status, exp, "channel status");

  // read data
  res = pamela_read_data(chn, buf);
  CHECK_PAM_RES_VAL(res, "read_data", TEST_BUF_SIZE);

  // close channel
  res = pamela_close(chn);
  CHECK_PAM_RES(res, "close");

  // check buffer
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

  return 0;
}

TEST_FUNC(test_write)
{
  pamela_handle_t *pam = (pamela_handle_t *)p->user_data;
  pamela_channel_t *chn = NULL;
  int res = 0;

  UBYTE *buf = test_buffer_alloc(TEST_BUF_SIZE, p);
  if (buf == NULL)
  {
    return 1;
  }

  for(int i=0;i<TEST_BUF_SIZE;i++) {
    UBYTE val = (UBYTE)((i + TEST_BYTE_OFFSET) & 0xff);
    buf[i] = val;
  }

  // open channel
  chn = pamela_open(pam, TEST_PORT, &res);
  CHECK_PAM_RES(res, "open");
  CHECK_NOT_NULL(chn, "open");

  // set mtu
  res = pamela_set_mtu(chn, TEST_BUF_SIZE);
  CHECK_PAM_RES(res, "set_mtu");
  UWORD mtu = 0;
  res = pamela_get_mtu(chn, &mtu);
  CHECK_EQUAL(mtu, TEST_BUF_SIZE, "mtu mismatch");

  // post write request
  res = pamela_write_request(chn, TEST_BUF_SIZE);
  CHECK_PAM_RES(res, "write_req");

  // wait for event
  res = pamela_event_wait(pam, 1, 0, NULL);
  CHECK_EQUAL(res, PAMELA_WAIT_EVENT, "wait: no event");

  // update state
  res = pamela_event_update(pam, NULL);
  CHECK_PAM_RES(res, "event_update");

  // check status
  UWORD status = pamela_status(chn);
  UWORD exp = PAMELA_STATUS_ACTIVE | PAMELA_STATUS_WRITE_READY;
  CHECK_EQUAL(status, exp, "channel status");

  // write data
  res = pamela_write_data(chn, buf);
  CHECK_PAM_RES_VAL(res, "write_data", TEST_BUF_SIZE);

  // close channel
  res = pamela_close(chn);
  CHECK_PAM_RES(res, "close");

  test_buffer_free(buf);

  return 0;
}
