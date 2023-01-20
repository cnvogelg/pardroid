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

#include "pamlib.h"
#include "pamlib-testsuite.h"

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

#define CHECK_PAMLIB_RES_WIRE_ERROR(res, sec, wire_error) \
  if (res != PAMELA_ERROR_WIRE) \
  { \
    p->error = pamela_perror(res); \
    p->section = sec; \
    return res; \
  } \
  if(pamlib_wire_error(ph) != wire_error) { \
    p->error = "wire error mismatch"; \
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

  // open channel
  pamlib_channel_t *ch = pamlib_open(ph, p->port, &res);
  CHECK_PAMLIB_RES(res, "open");

  // close request
  res = pamlib_close(ch);
  CHECK_PAMLIB_RES(res, "close");

  return 0;
}

TEST_FUNC(test_open_port_error)
{
  pamlib_handle_t *ph = (pamlib_handle_t *)p->user_data;
  int res = 0;

  // open channel
  pamlib_channel_t *ch = pamlib_open(ph, TEST_INVALID_PORT, &res);
  CHECK_PAMLIB_RES_VAL(res, "open", PAMELA_WIRE_ERROR_NO_SERVICE);

  return 0;
}

TEST_FUNC(test_open_own_error)
{
  pamlib_handle_t *ph = (pamlib_handle_t *)p->user_data;
  int res = 0;

  // open channel
  pamlib_channel_t *ch = pamlib_open(ph, TEST_OPEN_ERROR_PORT, &res);
  CHECK_PAMLIB_RES_VAL(res, "open", PAMELA_WIRE_ERROR_OPEN);

  return 0;
}

static int test_read_helper(test_param_t *p, UWORD read_size)
{
  pamlib_handle_t *ph = (pamlib_handle_t *)p->user_data;
  int res = 0;

  UBYTE *buf = test_buffer_alloc(TEST_MAX_BUF_SIZE, p);
  if (buf == NULL)
  {
    return 1;
  }

  // open channel
  pamlib_channel_t *ch = pamlib_open(ph, p->port, &res);
  CHECK_PAMLIB_RES(res, "open");

  // read
  res = pamlib_read(ch, buf, read_size);

  // error during req
  int check = 1;
  if((read_size == TEST_ERROR_REQ_SIZE) || (read_size == TEST_ERROR_POLL_SIZE)) {
    CHECK_PAMLIB_RES_WIRE_ERROR(res, "read", TEST_ERROR_READ);
    check = 0;
  } else {
    CHECK_PAMLIB_RES_VAL(res, "read", read_size);
  }

  // close channel
  res = pamlib_close(ch);
  CHECK_PAMLIB_RES(res, "close");

  // check buffer
  if(check) {
    int errors = 0;
    for(int i=0;i<read_size;i++) {
      UBYTE val = (UBYTE)((i + TEST_BYTE_OFFSET) & 0xff);
      if (buf[i] != val)
      {
        p->error = "value mismatch";
        p->section = "compare";
        sprintf(p->extra, "@%ld: w=%04lx r=%04lx (errors=%d)", (LONG)i, (ULONG)val, (ULONG)buf[i], errors);
        errors ++;
      }
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

TEST_FUNC(test_read_multi)
{
  return test_read_helper(p, TEST_MAX_BUF_SIZE - 2);
}

TEST_FUNC(test_read_multi_odd)
{
  return test_read_helper(p, TEST_MAX_BUF_SIZE - 3);
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
  pamlib_handle_t *ph = (pamlib_handle_t *)p->user_data;
  int res = 0;

  UBYTE *buf = test_buffer_alloc(TEST_MAX_BUF_SIZE, p);
  if (buf == NULL)
  {
    return 1;
  }

  for(int i=0;i<write_size;i++) {
    UBYTE val = (UBYTE)((i + TEST_BYTE_OFFSET) & 0xff);
    buf[i] = val;
  }

  // open channel
  pamlib_channel_t *ch = pamlib_open(ph, p->port, &res);
  CHECK_PAMLIB_RES(res, "open");

  // write
  res = pamlib_write(ch, buf, write_size);

  // error during req
  if((write_size == TEST_ERROR_REQ_SIZE) || (write_size == TEST_ERROR_POLL_SIZE)) {
    CHECK_PAMLIB_RES_WIRE_ERROR(res, "write", TEST_ERROR_WRITE);
  } else {
    CHECK_PAMLIB_RES_VAL(res, "write", write_size);
  }

  // close channel
  res = pamlib_close(ch);
  CHECK_PAMLIB_RES(res, "close");

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

TEST_FUNC(test_write_multi)
{
  return test_write_helper(p, TEST_MAX_BUF_SIZE - 2);
}

TEST_FUNC(test_write_multi_odd)
{
  return test_write_helper(p, TEST_MAX_BUF_SIZE - 3);
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
  pamlib_handle_t *ph = (pamlib_handle_t *)p->user_data;
  int res = 0;

  // open channel
  pamlib_channel_t *ch = pamlib_open(ph, p->port, &res);
  CHECK_PAMLIB_RES(res, "open");

  // seek
  res = pamlib_seek(ch, TEST_SEEK);
  CHECK_PAMLIB_RES(res, "seek");

  // tell
  ULONG pos = 0;
  res = pamlib_tell(ch, &pos);
  CHECK_PAMLIB_RES(res, "tell");

  // check pos
  if(pos != TEST_SEEK) {
    p->error = "wrong seek pos";
    p->section = "tell";
    sprintf(p->extra, "want=%lu, got=%lu", TEST_SEEK, pos);
    return 1;
  }

  // close channel
  res = pamlib_close(ch);
  CHECK_PAMLIB_RES(res, "close");

  return 0;
}

TEST_FUNC(test_get_set_mtu)
{
  pamlib_handle_t *ph = (pamlib_handle_t *)p->user_data;
  int res = 0;

  // open channel
  pamlib_channel_t *ch = pamlib_open(ph, p->port, &res);
  CHECK_PAMLIB_RES(res, "open");

  // get mtu
  UWORD mtu = 0;
  res = pamlib_get_mtu(ch, &mtu);
  CHECK_PAMLIB_RES(res, "get_mtu");

  if(mtu != TEST_DEFAULT_MTU) {
    p->error = "wrong default MTU";
    p->section = "get_mtu";
    sprintf(p->extra, "want=%u, got=%u", TEST_DEFAULT_MTU, mtu);
    return 1;
  }

  // set mtu
  #define TEST_MTU  128
  res = pamlib_set_mtu(ch, TEST_MTU);
  CHECK_PAMLIB_RES(res, "set_mtu");

  // get mtu
  res = pamlib_get_mtu(ch, &mtu);
  CHECK_PAMLIB_RES(res, "get_mtu");

  // check new mtu
  if(mtu != TEST_MTU) {
    p->error = "wrong set MTU";
    p->section = "set_mtu";
    sprintf(p->extra, "want=%u, got=%u", TEST_MTU, mtu);
    return 1;
  }

  // close channel
  res = pamlib_close(ch);
  CHECK_PAMLIB_RES(res, "close");

  return 0;
}
