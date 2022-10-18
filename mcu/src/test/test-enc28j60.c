#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG 1

#include "debug.h"

#include "uartutil.h"
#include "rominfo.h"
#include "fwid.h"
#include "fw_info.h"

#include "hw_uart.h"
#include "hw_system.h"
#include "hw_led.h"
#include "hw_timer.h"
#include "hw_spi.h"
#include "hw_dev.h"

#include "enc28j60.h"

FW_INFO(FWID_TEST_ENC28J60, VERSION_TAG)

#define PACKET_SIZE 512

u08 buf[PACKET_SIZE];

static void eth_ram_test(void)
{
  uart_send_pstring(PSTR("ram test:"));
  enc28j60_test_setup();

  // fill buffer
  u16 i;
  for(i=0;i<PACKET_SIZE;i++) {
    buf[i] = (u08)i;
  }

  // write buffer
  enc28j60_test_begin_tx();
  for(i=0;i<PACKET_SIZE;i++) {
    hw_spi_out(buf[i]);
  }
  enc28j60_test_end_tx();

  // read buffer
  enc28j60_test_begin_rx();
  for(i=0;i<PACKET_SIZE;i++) {
    buf[i] = hw_spi_in();
  }
  enc28j60_test_end_rx();

  // check buffer
  u16 errors = 0;
  for(i=0;i<PACKET_SIZE;i++) {
    u08 val = (u08)i;
    if(buf[i] != val) {
      uart_send_pstring(PSTR("MISMATCH @"));
      uart_send_hex_word(i);
      uart_send_pstring(PSTR(": "));
      uart_send_hex_word(buf[i]);
      uart_send_pstring(PSTR(" != "));
      uart_send_hex_byte(val);
      uart_send_crlf();
      errors++;
    }
  }
  if(errors == 0) {
    uart_send_pstring(PSTR("ok"));
  } else {
    uart_send_hex_word(errors);
    uart_send_pstring(PSTR(" ERRORS"));
  }
  uart_send_crlf();
}

static void eth_test(u08 partial)
{
  u08 mac[6] = { 0x1a,0x11,0xaf,0xa0,0x47,0x11 };

  // eth open
  uart_send_pstring(PSTR("eth open: "));
  u08 ok = enc28j60_setup(mac, ENC28J60_FLAGS_BROADCAST);
  uart_send_hex_byte(ok);
  enc28j60_enable();
  uart_send_crlf();

  uart_send_pstring(PSTR("wait for link"));
  for(u08 i=0;i<20;i++) {
    u08 up = enc28j60_is_link_up();
    if(up) {
      uart_send('*');
      break;
    } else {
      uart_send('.');
    }
    hw_timer_delay_ms(200);
  }
  uart_send_crlf();

  for(u16 i=0;i<PACKET_SIZE;i++) {
    buf[i] = (u08)(i & 0xff);
  }

  // send packets
  for(u08 i=0;i<100;i++) {
    buf[0] = i;

    uart_send_pstring(PSTR("eth send: "));
    if(partial) {
      uart_send_pstring(PSTR("partial: "));
    }
    uart_send_hex_byte(i);
    uart_send(':');

    // send packet
    if(partial) {
      const u16 off = PACKET_SIZE / 2;
      enc28j60_send_begin();
      enc28j60_send_data(buf, off);
      enc28j60_send_data(&buf[off], off);
      enc28j60_send_end(PACKET_SIZE);
    } else {
      enc28j60_send(buf, PACKET_SIZE);
    }

    // wait for packet
    hw_timer_ms_t t0 = hw_timer_millis();
    u08 pending = 0;
    while(1) {
      pending = enc28j60_get_pending_packets();
      if(pending) {
        break;
      }
      if(hw_timer_millis_timed_out(t0, 200)) {
        break;
      }
    }

    // incoming?
    if(pending) {
      u16 in_size;
      if(partial) {
        ok = enc28j60_recv_begin(&in_size);
        if(ok == ENC28J60_RESULT_OK) {
          const u16 off = PACKET_SIZE / 2;
          enc28j60_recv_data(buf, off);
          enc28j60_recv_data(&buf[off], off);
          enc28j60_recv_end();
        }
      } else {
        ok = enc28j60_recv(buf, PACKET_SIZE, &in_size);
      }
      uart_send('=');
      uart_send_hex_byte(ok);
      uart_send(',');
      uart_send_hex_word(in_size);
      uart_send(':');

      // show eth header
      for(u16 i=0; i<18; i++) {
        uart_send_hex_byte(buf[i]);
        uart_send(' ');
      }
    }
    uart_send_crlf();
  }

  // eth close
  uart_send_pstring(PSTR("eth close: "));
  enc28j60_disable();
  uart_send_crlf();
}

int main(void)
{
  hw_system_init();
  hw_dev_init();
  hw_spi_init();
  //led_init();

  hw_uart_init();
  uart_send_pstring(PSTR("parbox: test-enc28j60!"));
  uart_send_crlf();

  rom_info();

  uart_send_pstring(PSTR("enc28j60: init rev="));
  u08 rev;
  u08 ok = enc28j60_init(&rev);
  uart_send_hex_byte(rev);
  uart_send_pstring(PSTR(" ok="));
  uart_send_hex_byte(ok);
  uart_send_crlf();
  if(ok != ENC28J60_RESULT_OK) {
    goto end;
  }

  for(u08 i=0;i<5;i++) {
    eth_ram_test();
  }

  eth_test(0);

end:
  for(u08 i=0;i<100;i++) {
    uart_send('.');
    hw_timer_delay_ms(100);
  }

  uart_send_pstring(PSTR("reset..."));
  uart_send_crlf();
  hw_system_reset();
  return 0;
}
