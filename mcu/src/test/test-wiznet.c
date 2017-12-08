#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG 1

#include "debug.h"

#include "uart.h"
#include "uartutil.h"
#include "rominfo.h"
#include "system.h"
#include "led.h"
#include "timer.h"

#include "spi.h"
#include "wiznet.h"

#define PACKET_SIZE 512

//#define UDP_TEST
#define ETH_TEST

u08 buf[PACKET_SIZE];

#ifdef UDP_TEST
static void udp_test(u08 partial)
{
  // get socket
  uart_send_pstring(PSTR("udp socket: "));
  u08 sock = wiznet_find_free_socket();
  uart_send_hex_byte(sock);
  uart_send_crlf();

  // udp open
  uart_send_pstring(PSTR("udp open: "));
  u08 ok = wiznet_udp_open(sock, 4242);
  uart_send_hex_byte(ok);
  uart_send_crlf();

  for(u16 i=0;i<PACKET_SIZE;i++) {
    buf[i] = (u08)(i & 0xff);
  }
  wiz_udp_pkt_t pkt = {
    .addr = { 192,168,2,100 },
    .port = 4242,
    .len = PACKET_SIZE
  };

  // send packets
  uart_send_pstring(PSTR("udp send: "));
  for(u08 i=0;i<100;i++) {
    buf[0] = i;

    // send packet
    if(partial) {
      const u16 off = PACKET_SIZE / 2;
      wiznet_udp_send_begin(sock, &pkt);
      wiznet_udp_send_data(sock, 0, buf, off);
      wiznet_udp_send_data(sock, off, &buf[off], off);
      ok = wiznet_udp_send_end(sock, &pkt);

    } else {
      ok = wiznet_udp_send(sock, buf, &pkt);
    }
    uart_send_hex_byte(ok);
    uart_send('.');

    // wait for packet
    timer_ms_t t0 = timer_millis();
    u08 pending = 0;
    while(1) {
      pending = wiznet_udp_is_recv_pending(sock, 0);
      if(pending) {
        break;
      }
      if(timer_millis_timed_out(t0, 200)) {
        break;
      }
      system_wdt_reset();
    }

    // receive packet
    if(pending) {
      wiz_udp_pkt_t pkt_in;
      if(partial) {
        wiznet_udp_recv_begin(sock, &pkt_in);
        const u16 off = PACKET_SIZE / 2;
        wiznet_udp_recv_data(sock, 0, buf, off);
        wiznet_udp_recv_data(sock, off, &buf[off], off);
        ok = wiznet_udp_recv_end(sock, &pkt_in);
      } else {
        ok = wiznet_udp_recv(sock, buf, PACKET_SIZE, &pkt_in);
      }
      uart_send('=');
      uart_send_hex_byte(ok);
      uart_send(',');
      uart_send_hex_word(pkt_in.len);

      // check seq num
      if(buf[0] != i) {
        uart_send('!');
        uart_send_crlf();
        for(u16 i=0; i<PACKET_SIZE; i++) {
          uart_send_hex_byte(buf[i]);
          uart_send(' ');
        }
      }
    }
    uart_send(',');
  }

  // udp close
  uart_send_pstring(PSTR("udp close: "));
  ok = wiznet_udp_close(sock);
  uart_send_hex_byte(ok);
  uart_send_crlf();
}
#endif

#ifdef ETH_TEST
static void eth_test(u08 partial)
{
  // eth open
  uart_send_pstring(PSTR("eth open: "));
  u08 ok = wiznet_eth_open(1);
  uart_send_hex_byte(ok);
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
      ok = wiznet_eth_send_begin(PACKET_SIZE);
      if(ok == WIZNET_RESULT_OK) {
        wiznet_eth_send_data(0, buf, off);
        wiznet_eth_send_data(off, &buf[off], off);
        ok = wiznet_eth_send_end(PACKET_SIZE);
      }
    } else {
      ok = wiznet_eth_send(buf, PACKET_SIZE);
    }
    uart_send_hex_byte(ok);
    uart_send('+');

    // wait for packet
    timer_ms_t t0 = timer_millis();
    u08 pending = 0;
    while(1) {
      pending = wiznet_eth_is_recv_pending();
      if(pending) {
        break;
      }
      if(timer_millis_timed_out(t0, 200)) {
        break;
      }
      system_wdt_reset();
    }

    // incoming?
    if(pending) {
      u16 in_size;
      if(partial) {
        wiznet_eth_recv_begin(&in_size);
        const u16 off = PACKET_SIZE / 2;
        wiznet_eth_recv_data(0, buf, off);
        wiznet_eth_recv_data(off, &buf[off], off);
        ok = wiznet_eth_recv_end(in_size);
      } else {
        ok = wiznet_eth_recv(buf, PACKET_SIZE, &in_size);
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
  ok = wiznet_eth_close();
  uart_send_hex_byte(ok);
  uart_send_crlf();
}
#endif

int main(void)
{
  system_init();
  //led_init();

  uart_init();
  uart_send_pstring(PSTR("parbox: test-base!"));
  uart_send_crlf();

  rom_info();

  spi_init();

  uart_send_pstring(PSTR("wiznet: init"));
  wiznet_init();

  u08 mac[6] = { 0x1a,0x11,0xaf,0xa0,0x47,0x11 };
  wiznet_set_mac(mac);
  u08 addr[4] = { 192,168,2,42 };
  wiznet_set_src_addr(addr);
  u08 mask[4] = { 255,255,255,0 };
  wiznet_set_net_mask(mask);
  u08 gw[4] = { 192,168,2,1 };
  wiznet_set_gw_addr(gw);

  uart_send_crlf();

  uart_send_pstring(PSTR("wait for link"));
  while(1) {
    system_wdt_reset();
    u08 up = wiznet_is_link_up();
    if(up) {
      uart_send('*');
      break;
    } else {
      uart_send('.');
    }
    timer_delay(200);
  }
  uart_send_crlf();

#ifdef UDP_TEST
  udp_test(0);
  udp_test(1);
#endif
#ifdef ETH_TEST
  eth_test(0);
  eth_test(1);
#endif

  for(u08 i=0;i<20;i++) {
    system_wdt_reset();
    uart_send('.');
    timer_delay(100);
  }

  uart_send_pstring(PSTR("reset..."));
  uart_send_crlf();
  system_sys_reset();
  return 0;
}
