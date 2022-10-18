#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "debug.h"

#include "hw_system.h"
#include "hw_led.h"
#include "hw_timer.h"
#include "hw_uart.h"

#include "uartutil.h"
#include "rominfo.h"

#include "proto_io.h"
#include "proto//wire_io.h"
#include "pamela/wire.h"

#include "fwid.h"
#include "fw_info.h"

FW_INFO(FWID_TEST_PROTO_IO, VERSION_TAG)

#define BUF_SIZE    512

static u32 test_offset;
static u16 test_status;
static u16 test_mtu;
static u16 read_size;
static u16 write_size;
static u08 data_buf[BUF_SIZE];

// API

u16  proto_io_api_get_default_mtu(void)
{
  uart_send_pstring(PSTR("default mtu"));
  uart_send_crlf();

  return 0x1234;
}

u16 proto_io_api_get_max_channels(void)
{
  uart_send_pstring(PSTR("max channels"));
  uart_send_crlf();

  return PROTO_IO_NUM_CHANNELS;
}

u16 proto_io_api_get_channel_mtu(u08 chn)
{
  uart_send_pstring(PSTR("get_mtu:#"));
  uart_send_hex_byte(chn);
  uart_send(',');
  uart_send_hex_word(test_mtu);
  uart_send_crlf();

  return test_mtu;
}

void proto_io_api_set_channel_mtu(u08 chn, u16 mtu)
{
  uart_send_pstring(PSTR("set_mtu:#"));
  uart_send_hex_byte(chn);
  uart_send(',');
  uart_send_hex_word(mtu);
  uart_send_crlf();

  test_mtu = mtu;
}

void proto_io_api_open(u08 chn, u16 port)
{
  uart_send_pstring(PSTR("open:#"));
  uart_send_hex_byte(chn);
  uart_send(',');
  uart_send_hex_word(port);
  uart_send_crlf();

  // we misuse the mtu to pass port num
  test_mtu = port;
  test_status = PAMELA_STATUS_ACTIVE;
  proto_io_event_mask_add_chn(chn);
}

void proto_io_api_close(u08 chn)
{
  uart_send_pstring(PSTR("close:#"));
  uart_send_hex_byte(chn);
  uart_send_crlf();

  test_status = 0;
  proto_io_event_mask_add_chn(chn);
}

extern void proto_io_api_reset(u08 chn)
{
  uart_send_pstring(PSTR("reset:#"));
  uart_send_hex_byte(chn);
  uart_send_crlf();

  test_status = PAMELA_STATUS_ACTIVE;
  proto_io_event_mask_add_chn(chn);
}

void proto_io_api_seek(u08 chn, u32 off)
{
  uart_send_pstring(PSTR("seek:#"));
  uart_send_hex_byte(chn);
  uart_send(',');
  uart_send_hex_long(off);
  uart_send_crlf();

  test_offset = off;
}

u32 proto_io_api_tell(u08 chn)
{
  uart_send_pstring(PSTR("tell:#"));
  uart_send_hex_byte(chn);
  uart_send(',');
  uart_send_hex_long(test_offset);
  uart_send_crlf();

  return test_offset;
} 

u16  proto_io_api_status(u08 chn)
{
  return test_status;
}

// read

void proto_io_api_read_req(u08 chn, u16 size)
{
  read_size = size;

  test_status |= PAMELA_STATUS_READ_REQ;
  proto_io_event_mask_add_chn(chn);
}

u16  proto_io_api_read_res(u08 chn)
{
  return read_size;
}

void proto_io_api_read_blk(u08 chn, u16 *size, u08 **buf)
{
  *size = read_size;
  *buf = data_buf;
}

void proto_io_api_read_done(u08 chn, u16 size, u08 *buf)
{
}

// write

void proto_io_api_write_req(u08 chn, u16 size)
{
  write_size = size;

  test_status |= PAMELA_STATUS_WRITE_REQ;
  proto_io_event_mask_add_chn(chn);
}

u16  proto_io_api_write_res(u08 chn)
{
  return write_size;
}

void proto_io_api_write_blk(u08 chn, u16 *size, u08 **buf)
{
  *size = write_size;
  *buf = data_buf;
}

void proto_io_api_write_done(u08 chn, u16 size, u08 *buf)
{
}

// ----- main -----

int main(void)
{
  hw_system_init();
  hw_led_init();
  hw_uart_init();

  uart_send_pstring(PSTR("\r\n\r\n-----\r\nparbox: test-proto-io!"));
  uart_send_crlf();

  rom_info();

  proto_io_init();

  while(1) {
    // handle all proto io commands
    proto_io_handle_cmd();
  }

  return 0;
}
