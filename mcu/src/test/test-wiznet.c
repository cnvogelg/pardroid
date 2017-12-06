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

#define UDP_PACKET_SIZE 512

u08 buf[UDP_PACKET_SIZE];


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

  // get socket
  uart_send_pstring(PSTR("socket: "));
  u08 sock = wiznet_find_free_socket();
  uart_send_hex_byte(sock);
  uart_send_crlf();

  // udp open
  uart_send_pstring(PSTR("udp open: "));
  u08 ok = wiznet_udp_open(sock, 4242);
  uart_send_hex_byte(ok);
  uart_send_crlf();

  for(u16 i=0;i<UDP_PACKET_SIZE;i++) {
    buf[i] = (u08)(i & 0xff);
  }
  u08 dst_ip[4] = { 192,168,2,100 };
  u16 dst_port = 4242;

  // send packets
  uart_send_pstring(PSTR("udp send: "));
  for(u08 i=0;i<100;i++) {
    buf[0] = i;
    ok = wiznet_udp_send(sock, buf, UDP_PACKET_SIZE, dst_ip, dst_port);
    uart_send_hex_byte(ok);
    uart_send('.');
    system_wdt_reset();
    timer_delay(200);
  }

  // udp close
  uart_send_pstring(PSTR("udp close: "));
  ok = wiznet_udp_close(sock);
  uart_send_hex_byte(ok);
  uart_send_crlf();

  uart_send_pstring(PSTR("reset..."));
  uart_send_crlf();
  system_sys_reset();
  return 0;
}
