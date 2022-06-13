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

#include "devices/pamela.h"
#include "pamela.h"
#include "pamela_engine.h"
#include "pamela-engine-testsuite.h"

#define CHECK_PAM_REQ(res, pet, sec) \
  res = handle_req(pet); \
  if (res != 0) \
  { \
    p->error = pamela_perror(res); \
    p->section = sec; \
    return res; \
  }

#define CHECK_PAM_REQ_VAL(res, pet, sec, val) \
  res = handle_req(pet); \
  if (res != val) \
  { \
    p->error = pamela_perror(res); \
    p->section = sec; \
    return res; \
  }

#define TEST_BUF_SIZE  128
#define TEST_BYTE_OFFSET 3
#define TEST_SEEK       0xdeadbeefUL

static int handle_req(pam_eng_test_data_t *pet)
{
  BOOL quick = pamela_engine_post_request(pet->engine, pet->req);
  if(quick) {
    // check pamela error
    int error = pet->req->iopam_PamelaError;
    if(error < 0) {
      Printf("  quick: Pamela error: %ld = %s\n", error, pamela_perror(error));
    }
    return error;
  } else {
    // let the engine work
    ULONG extra_sigmask = 1 << pet->port->mp_SigBit;
    ULONG sigmask = pamela_engine_work(pet->engine, extra_sigmask);
    if(sigmask != extra_sigmask) {
      Printf("  Returned wrong sigmask: %08lx\n", sigmask);
      return -99;
    }
    // get request back
    struct IOPamReq *res_req = (struct IOPamReq *)GetMsg(pet->port);
    if(res_req != pet->req) {
      Printf("  Returned wrong request! %08lx\n", res_req);
      return -99;
    }
    // check pamela error
    int error = pet->req->iopam_PamelaError;
    if(error < 0) {
      Printf("  Pamela error: %ld = %s\n", error, pamela_perror(error));
    }
    return error;
  }
}

TEST_FUNC(test_init_exit)
{
  // nothing to do. init/exit is done in all tests.
  return 0;
}

TEST_FUNC(test_open_close)
{
  pam_eng_test_data_t *pet = (pam_eng_test_data_t *)p->user_data;
  pamela_req_t *req = pet->req;
  int res = 0;

  // open request
  req->iopam_Req.io_Command = PAMCMD_OPEN_CHANNEL;
  req->iopam_Port = p->port;
  CHECK_PAM_REQ(res, pet, "open");

  // close request
  req->iopam_Req.io_Command = PAMCMD_CLOSE_CHANNEL;
  CHECK_PAM_REQ(res, pet, "close");

  return 0;
}

TEST_FUNC(test_read)
{
  pam_eng_test_data_t *pet = (pam_eng_test_data_t *)p->user_data;
  pamela_req_t *req = pet->req;
  int res = 0;

  UBYTE *buf = test_buffer_alloc(TEST_BUF_SIZE, p);
  if (buf == NULL)
  {
    return 1;
  }

  // open channel
  req->iopam_Req.io_Command = PAMCMD_OPEN_CHANNEL;
  req->iopam_Port = p->port;
  CHECK_PAM_REQ(res, pet, "open");

  // read
  req->iopam_Req.io_Command = PAMCMD_READ;
  req->iopam_Req.io_Length = TEST_BUF_SIZE;
  req->iopam_Req.io_Data = buf;
  CHECK_PAM_REQ_VAL(res, pet, "read", TEST_BUF_SIZE);

  // close channel
  req->iopam_Req.io_Command = PAMCMD_CLOSE_CHANNEL;
  CHECK_PAM_REQ(res, pet, "close");

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
  pam_eng_test_data_t *pet = (pam_eng_test_data_t *)p->user_data;
  pamela_req_t *req = pet->req;
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
  req->iopam_Req.io_Command = PAMCMD_OPEN_CHANNEL;
  req->iopam_Port = p->port;
  CHECK_PAM_REQ(res, pet, "open");

  // write
  req->iopam_Req.io_Command = PAMCMD_WRITE;
  req->iopam_Req.io_Length = TEST_BUF_SIZE;
  req->iopam_Req.io_Data = buf;
  CHECK_PAM_REQ_VAL(res, pet, "read", TEST_BUF_SIZE);

  // close channel
  req->iopam_Req.io_Command = PAMCMD_CLOSE_CHANNEL;
  CHECK_PAM_REQ(res, pet, "close");

  test_buffer_free(buf);

  return 0;
}

TEST_FUNC(test_seek_tell)
{
  pam_eng_test_data_t *pet = (pam_eng_test_data_t *)p->user_data;
  pamela_req_t *req = pet->req;
  int res = 0;

  // open channel
  req->iopam_Req.io_Command = PAMCMD_OPEN_CHANNEL;
  req->iopam_Port = p->port;
  CHECK_PAM_REQ(res, pet, "open");

  // seek
  req->iopam_Req.io_Command = PAMCMD_SEEK;
  req->iopam_Req.io_Offset = TEST_SEEK;
  CHECK_PAM_REQ(res, pet, "seek");

  // clear
  req->iopam_Req.io_Offset = 0;

  // tell
  req->iopam_Req.io_Command = PAMCMD_TELL;
  CHECK_PAM_REQ(res, pet, "tell");

  // check pos
  ULONG pos = req->iopam_Req.io_Offset;
  if(pos != TEST_SEEK) {
    p->error = "wrong seek pos";
    p->section = "tell";
    sprintf(p->extra, "want=%lu, got=%lu", TEST_SEEK, pos);
    return 1;
  }

  // close channel
  req->iopam_Req.io_Command = PAMCMD_CLOSE_CHANNEL;
  CHECK_PAM_REQ(res, pet, "close");

  return 0;
}
