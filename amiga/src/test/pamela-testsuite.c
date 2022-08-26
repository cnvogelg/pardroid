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

#include "test/pamela.h"

#define TEST_SEEK       0xdeadbeefUL

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

#define CHECK_WAIT_EVENT(ph, ch, sec) \
  res = wait_event(ph, ch); \
  if(res != PAMELA_OK) { \
    p->error = pamela_perror(res); \
    p->section = sec; \
    sprintf(p->extra, "wait_event"); \
    return res; \
  }

#define CHECK_CHANNEL_STATE(ch, sec, want) \
  { UWORD state = pamela_status(ch); \
    if(state != want) { \
      p->error = "invalid state"; \
      p->section = sec; \
      sprintf(p->extra, "got %04x want %04x", state, want); \
      return 1; \
    } \
  }

#define TEST_BUF_SIZE  512
#define TEST_BYTE_OFFSET 3

static int wait_event(pamela_handle_t *ph, pamela_channel_t *ch)
{
  int wait = pamela_event_wait(ph, 10, 0, NULL);
  if(wait == PAMELA_WAIT_EVENT) {
    UWORD mask = 0;
    return pamela_event_update(ph, &mask);
  } else {
    return PAMELA_ERROR_UNKNOWN;
  }
}

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

  chn = pamela_open(pam, p->port, &res);
  CHECK_PAM_RES(res, "open");
  CHECK_NOT_NULL(chn, "open");
  CHECK_WAIT_EVENT(pam, chn, "open");
  CHECK_CHANNEL_STATE(chn, "open", PAMELA_STATUS_ACTIVE);

  res = pamela_close(chn);
  CHECK_PAM_RES(res, "close");
  CHECK_WAIT_EVENT(pam, chn, "close");
  CHECK_CHANNEL_STATE(chn, "close", PAMELA_STATUS_INACTIVE);

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
  chn = pamela_open(pam, p->port, &res);
  CHECK_PAM_RES(res, "open");
  CHECK_NOT_NULL(chn, "open");
  CHECK_WAIT_EVENT(pam, chn, "open");
  CHECK_CHANNEL_STATE(chn, "open", PAMELA_STATUS_ACTIVE);

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
  CHECK_WAIT_EVENT(pam, chn, "close");
  CHECK_CHANNEL_STATE(chn, "close", PAMELA_STATUS_INACTIVE);

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
  chn = pamela_open(pam, p->port, &res);
  CHECK_PAM_RES(res, "open");
  CHECK_NOT_NULL(chn, "open");
  CHECK_WAIT_EVENT(pam, chn, "open");
  CHECK_CHANNEL_STATE(chn, "open", PAMELA_STATUS_ACTIVE);

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
  CHECK_WAIT_EVENT(pam, chn, "close");
  CHECK_CHANNEL_STATE(chn, "close", PAMELA_STATUS_INACTIVE);

  test_buffer_free(buf);

  return 0;
}

TEST_FUNC(test_seek_tell)
{
  pamela_handle_t *pam = (pamela_handle_t *)p->user_data;
  int res = 0;

  // open channel
  pamela_channel_t *chn = pamela_open(pam, p->port, &res);
  CHECK_PAM_RES(res, "open");
  CHECK_NOT_NULL(chn, "open");
  CHECK_WAIT_EVENT(pam, chn, "open");
  CHECK_CHANNEL_STATE(chn, "open", PAMELA_STATUS_ACTIVE);

  // seek
  res = pamela_seek(chn, TEST_SEEK);
  CHECK_PAM_RES(res, "seek");

  // tell
  ULONG pos = 0;
  res = pamela_tell(chn, &pos);
  CHECK_PAM_RES(res, "tell");

  // check pos
  if(pos != TEST_SEEK) {
    p->error = "wrong seek pos";
    p->section = "tell";
    sprintf(p->extra, "want=%lu, got=%lu", TEST_SEEK, pos);
    return 1;
  }

  // close channel
  res = pamela_close(chn);
  CHECK_PAM_RES(res, "close");

  return 0;
}

TEST_FUNC(test_get_set_mtu)
{
  pamela_handle_t *pam = (pamela_handle_t *)p->user_data;
  int res = 0;

  // open channel
  pamela_channel_t *chn = pamela_open(pam, p->port, &res);
  CHECK_PAM_RES(res, "open");
  CHECK_NOT_NULL(chn, "open");
  CHECK_WAIT_EVENT(pam, chn, "open");
  CHECK_CHANNEL_STATE(chn, "open", PAMELA_STATUS_ACTIVE);

  // get mtu
  UWORD mtu = 0;
  res = pamela_get_mtu(chn, &mtu);
  CHECK_PAM_RES(res, "get_mtu");

  if(mtu != TEST_DEFAULT_MTU) {
    p->error = "wrong default MTU";
    p->section = "get_mtu";
    sprintf(p->extra, "want=%u, got=%u", TEST_DEFAULT_MTU, mtu);
    return 1;
  }

  // set mtu
  #define TEST_MTU  128
  res = pamela_set_mtu(chn, TEST_MTU);
  CHECK_PAM_RES(res, "set_mtu");

  // get mtu
  res = pamela_get_mtu(chn, &mtu);
  CHECK_PAM_RES(res, "get_mtu");

  // check new mtu
  if(mtu != TEST_MTU) {
    p->error = "wrong set MTU";
    p->section = "set_mtu";
    sprintf(p->extra, "want=%u, got=%u", TEST_MTU, mtu);
    return 1;
  }

  // close channel
  res = pamela_close(chn);
  CHECK_PAM_RES(res, "close");

  return 0;
}
