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
#include "proto_io.h"
#include "proto-io-testsuite.h"
#include "proto_io_shared.h"
#include "test-buffer.h"

#define TEST_CHANNEL    7
#define TEST_PORT       54321
#define TEST_SEEK       0xdeadbeefUL
#define TEST_SIZE       4096
#define TEST_BUF_SIZE   512
#define TEST_BYTE_OFFSET 3

// ----- global config -----

TEST_FUNC(test_event_mask)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;
  UWORD event_mask = 0;

  // get initial mask: empty
  int res = proto_io_get_event_mask(proto, &event_mask);
  CHECK_RES(res, "get1");
  CHECK_EQUAL(event_mask, 0, "init empty");

  // reset a channel
  res = proto_io_reset(proto, TEST_CHANNEL);
  CHECK_RES(res, "reset");

  // get new mask
  res = proto_io_get_event_mask(proto, &event_mask);
  CHECK_RES(res, "get2");

  // assume bit set
  UWORD expect = 1 << TEST_CHANNEL;
  CHECK_EQUAL(event_mask, expect, "channel bit set");

  // get new mask
  res = proto_io_get_event_mask(proto, &event_mask);
  CHECK_RES(res, "get3");
  CHECK_EQUAL(event_mask, 0, "after empty");

  return 0;
}

TEST_FUNC(test_mtu)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;
  UWORD mtu = 0;

  // get default mtu
  int res = proto_io_get_default_mtu(proto, &mtu);
  CHECK_RES(res, "get_default_mtu");
  CHECK_EQUAL(mtu, 0x1234, "default mtu");

  // set channel mtu
  res = proto_io_set_channel_mtu(proto, TEST_CHANNEL, 0x2244);
  CHECK_RES(res, "set_channel_mtu");

  // get channel mtu
  res = proto_io_get_channel_mtu(proto, TEST_CHANNEL, &mtu);
  CHECK_RES(res, "get_channel_mtu");
  CHECK_EQUAL(mtu, 0x2244, "channel mtu");

  return 0;
}

TEST_FUNC(test_max_channels)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;
  UWORD max_chn = 0;

  // get max channels
  int res = proto_io_get_max_channels(proto, &max_chn);
  CHECK_RES(res, "get_max_channels");
  CHECK_EQUAL(max_chn, PROTO_IO_NUM_CHANNELS, "max channels");

  return 0;
}

TEST_FUNC(test_open_close)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;

  // open
  int res = proto_io_open(proto, TEST_CHANNEL, TEST_PORT);
  CHECK_RES(res, "open");

  // check status
  UWORD status = 0;
  res = proto_io_status(proto, TEST_CHANNEL, &status);
  CHECK_RES(res, "status1");
  CHECK_EQUAL(status, PROTO_IO_STATUS_OPEN, "open bit set");

  // check port
  UWORD port = 0;
  res = proto_io_get_channel_mtu(proto, TEST_CHANNEL, &port);
  CHECK_RES(res, "get_channel_mtu");
  CHECK_EQUAL(port, TEST_PORT, "open port");

  // close
  res = proto_io_close(proto, TEST_CHANNEL);
  CHECK_RES(res, "close");

  // check status
  res = proto_io_status(proto, TEST_CHANNEL, &status);
  CHECK_RES(res, "status1");
  CHECK_EQUAL(status, 0, "open bit cleared");

  // clear event mask
  UWORD mask = 0;
  res = proto_io_get_event_mask(proto, &mask);
  CHECK_RES(res, "event_mask");

  return 0;
}

TEST_FUNC(test_reset)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;
  UWORD status = 0;

  // get max channels
  int res = proto_io_reset(proto, TEST_CHANNEL);
  CHECK_RES(res, "reset");

  // check status: only open is set
  res = proto_io_status(proto, TEST_CHANNEL, &status);
  CHECK_RES(res, "status");
  CHECK_EQUAL(status, PROTO_IO_STATUS_OPEN, "reset bit set");

  // clear event mask
  UWORD mask = 0;
  res = proto_io_get_event_mask(proto, &mask);
  CHECK_RES(res, "event_mask");

  return 0;
}

TEST_FUNC(test_seek_tell)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;

  // seek
  int res = proto_io_seek(proto, TEST_CHANNEL, TEST_SEEK);
  CHECK_RES(res, "seek");

  // tell
  ULONG pos;
  res = proto_io_tell(proto, TEST_CHANNEL, &pos);
  CHECK_RES(res, "tell");
  CHECK_LEQUAL(pos, TEST_SEEK, "seek pos");

  return 0;
}

TEST_FUNC(test_read_req_res)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;

  // req
  int res = proto_io_read_request(proto, TEST_CHANNEL, TEST_SIZE);
  CHECK_RES(res, "req");

  // res
  UWORD size;
  res = proto_io_read_result(proto, TEST_CHANNEL, &size);
  CHECK_RES(res, "res");
  CHECK_EQUAL(size, TEST_SIZE, "size");

  // clear event mask
  UWORD mask = 0;
  res = proto_io_get_event_mask(proto, &mask);
  CHECK_RES(res, "event_mask");

  return 0;
}

TEST_FUNC(test_write_req_res)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;

  // req
  int res = proto_io_write_request(proto, TEST_CHANNEL, TEST_SIZE);
  CHECK_RES(res, "req");

  // res
  UWORD size;
  res = proto_io_write_result(proto, TEST_CHANNEL, &size);
  CHECK_RES(res, "res");
  CHECK_EQUAL(size, TEST_SIZE, "size");

  // clear event mask
  UWORD mask = 0;
  res = proto_io_get_event_mask(proto, &mask);
  CHECK_RES(res, "event_mask");

  return 0;
}

TEST_FUNC(test_rw_data)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;

  UBYTE *buf = test_buffer_alloc(TEST_BUF_SIZE, p);
  if (buf == NULL)
  {
    return 1;
  }
  for(int i=0;i<TEST_BUF_SIZE;i++) {
    UBYTE val = (UBYTE)((i + TEST_BYTE_OFFSET) & 0xff);
    buf[i] = val;
  }

  // set write size
  int res = proto_io_write_request(proto, TEST_CHANNEL, TEST_BUF_SIZE);
  CHECK_RES(res, "write_req");

  // write buffer
  res = proto_io_write_data(proto, TEST_CHANNEL, buf, TEST_BUF_SIZE);
  CHECK_RES(res, "write_data");

  // set read size
  res = proto_io_read_request(proto, TEST_CHANNEL, TEST_BUF_SIZE);
  CHECK_RES(res, "read_req");

  // read buffer
  res = proto_io_read_data(proto, TEST_CHANNEL, buf, TEST_BUF_SIZE);
  CHECK_RES(res, "read_data");

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

  if(errors > 0) {
    return 1;
  }
  
  // clear event mask
  UWORD mask = 0;
  res = proto_io_get_event_mask(proto, &mask);
  CHECK_RES(res, "event_mask");

  return 0;
}
