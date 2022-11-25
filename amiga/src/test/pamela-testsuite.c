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

TEST_FUNC(test_open_port_error)
{
  pamela_handle_t *pam = (pamela_handle_t *)p->user_data;
  pamela_channel_t *chn = NULL;
  int res = 0;

  chn = pamela_open(pam, TEST_INVALID_PORT, &res);
  CHECK_PAM_RES(res, "open");
  CHECK_NOT_NULL(chn, "open");
  CHECK_WAIT_EVENT(pam, chn, "open");
  CHECK_CHANNEL_STATE(chn, "open", PAMELA_STATUS_OPEN_ERROR);

  UWORD error = pamela_error(chn);
  CHECK_PAM_RES_VAL(error, "wire_error", PAMELA_WIRE_ERROR_NO_SERVICE);

  res = pamela_close(chn);
  CHECK_PAM_RES(res, "close");
  CHECK_WAIT_EVENT(pam, chn, "close");
  CHECK_CHANNEL_STATE(chn, "close", PAMELA_STATUS_INACTIVE);

  return 0;
}

TEST_FUNC(test_open_own_error)
{
  pamela_handle_t *pam = (pamela_handle_t *)p->user_data;
  pamela_channel_t *chn = NULL;
  int res = 0;

  chn = pamela_open(pam, TEST_OPEN_ERROR_PORT, &res);
  CHECK_PAM_RES(res, "open");
  CHECK_NOT_NULL(chn, "open");
  CHECK_WAIT_EVENT(pam, chn, "open");
  CHECK_CHANNEL_STATE(chn, "open", PAMELA_STATUS_OPEN_ERROR);

  UWORD error = pamela_error(chn);
  CHECK_PAM_RES_VAL(error, "wire_error", PAMELA_WIRE_ERROR_OPEN);

  res = pamela_close(chn);
  CHECK_PAM_RES(res, "close");
  CHECK_WAIT_EVENT(pam, chn, "close");
  CHECK_CHANNEL_STATE(chn, "close", PAMELA_STATUS_INACTIVE);

  return 0;
}

static int test_read_helper(test_param_t *p, UWORD read_size)
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
  res = pamela_read_request(chn, read_size);
  CHECK_PAM_RES(res, "read_req");

  // wait for event
  res = pamela_event_wait(pam, 1, 0, NULL);
  CHECK_EQUAL(res, PAMELA_WAIT_EVENT, "wait: no event");

  // update state
  res = pamela_event_update(pam, NULL);
  CHECK_PAM_RES(res, "event_update");

  // check status
  UWORD status = pamela_status(chn);

  // error during req
  if((read_size == TEST_ERROR_REQ_SIZE) || (read_size == TEST_ERROR_POLL_SIZE)) {
     UWORD exp = PAMELA_STATUS_ERROR;
     CHECK_EQUAL(status, exp, "channel status");
     CHECK_PAM_RES_VAL(pamela_error(chn), "error code", TEST_ERROR_READ);
  } else {
    UWORD exp = PAMELA_STATUS_ACTIVE | PAMELA_STATUS_READ_READY;
    CHECK_EQUAL(status, exp, "channel status");

    // read data
    res = pamela_read_data(chn, buf);
    CHECK_PAM_RES_VAL(res, "read_data", read_size);

    // error after read
    if(read_size == TEST_ERROR_DONE_SIZE) {
      CHECK_WAIT_EVENT(pam, chn, "after read");
      CHECK_CHANNEL_STATE(chn, "after read", PAMELA_STATUS_ERROR);
      CHECK_PAM_RES_VAL(pamela_error(chn), "error code", TEST_ERROR_READ);
    }
  }

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

TEST_FUNC(test_read)
{
  return test_read_helper(p, TEST_BUF_SIZE);
}

TEST_FUNC(test_read_odd)
{
  return test_read_helper(p, TEST_BUF_SIZE - 1);
}

TEST_FUNC(test_read_error_req)
{
  return test_read_helper(p, TEST_ERROR_REQ_SIZE);
}

TEST_FUNC(test_read_error_poll)
{
  return test_read_helper(p, TEST_ERROR_POLL_SIZE);
}

TEST_FUNC(test_read_error_done)
{
  return test_read_helper(p, TEST_ERROR_DONE_SIZE);
}

static int test_write_helper(test_param_t *p, UWORD write_size)
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
  res = pamela_write_request(chn, write_size);
  CHECK_PAM_RES(res, "write_req");

  // wait for event
  res = pamela_event_wait(pam, 1, 0, NULL);
  CHECK_EQUAL(res, PAMELA_WAIT_EVENT, "wait: no event");

  // update state
  res = pamela_event_update(pam, NULL);
  CHECK_PAM_RES(res, "event_update");

  // check status
  UWORD status = pamela_status(chn);

  // error during req
  if((write_size == TEST_ERROR_REQ_SIZE) || (write_size == TEST_ERROR_POLL_SIZE)) {
     UWORD exp = PAMELA_STATUS_ERROR;
     CHECK_EQUAL(status, exp, "channel status");
     CHECK_PAM_RES_VAL(pamela_error(chn), "error code", TEST_ERROR_WRITE);
  } else {
    UWORD exp = PAMELA_STATUS_ACTIVE | PAMELA_STATUS_WRITE_READY;
    CHECK_EQUAL(status, exp, "channel status");

    // write data
    res = pamela_write_data(chn, buf);
    CHECK_PAM_RES_VAL(res, "write_data", write_size);

    // error after write
    if(write_size == TEST_ERROR_DONE_SIZE) {
      CHECK_WAIT_EVENT(pam, chn, "after write");
      CHECK_CHANNEL_STATE(chn, "after write", PAMELA_STATUS_ERROR);
      CHECK_PAM_RES_VAL(pamela_error(chn), "error code", TEST_ERROR_WRITE);
    }
  }

  // close channel
  res = pamela_close(chn);
  CHECK_PAM_RES(res, "close");
  CHECK_WAIT_EVENT(pam, chn, "close");
  CHECK_CHANNEL_STATE(chn, "close", PAMELA_STATUS_INACTIVE);

  test_buffer_free(buf);

  return 0;
}

TEST_FUNC(test_write)
{
  return test_write_helper(p, TEST_BUF_SIZE);
}

TEST_FUNC(test_write_odd)
{
  return test_write_helper(p, TEST_BUF_SIZE - 1);
}

TEST_FUNC(test_write_error_req)
{
  return test_write_helper(p, TEST_ERROR_REQ_SIZE);
}

TEST_FUNC(test_write_error_poll)
{
  return test_write_helper(p, TEST_ERROR_POLL_SIZE);
}

TEST_FUNC(test_write_error_done)
{
  return test_write_helper(p, TEST_ERROR_DONE_SIZE);
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
