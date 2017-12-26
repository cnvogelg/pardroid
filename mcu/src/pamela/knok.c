#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG CONFIG_DEBUG_KNOK

#include "debug.h"

#include "knok.h"
#include "strobe.h"
#include "uart.h"
#include "uartutil.h"
#include "system.h"
#include "timer.h"
#include "led.h"

// get generated bootstrap code
#include "bootstrap.h"
#include "bootrexx.h"

#define SEND_OK                  0
#define SEND_NOT_STARTED         1
#define SEND_ABORTED             2
#define SEND_NOT_FINISHED        3

static u08 knok_upload(rom_pchar data, u16 size, rom_pchar title)
{
  u08 res = SEND_OK;

  uart_send_pstring(PSTR("upload:@"));
  uart_send_hex_ptr(data);
  uart_send('+');
  uart_send_hex_word(size);
  uart_send(':');
  uart_send_pstring(title);

  strobe_send_begin(data, size);

  u16 interval = 200;
  u08 flag = STROBE_FLAG_NONE;
  const u08 max = 100;

  // wait for first strobe
  strobe_pulse_ack();
  uart_send('.');

  timer_ms_t t0 = timer_millis();
  u08 count = 0;
  u08 led = 1;
  while(1) {
    if(timer_millis_timed_out(t0, interval)) {
      flag = strobe_read_flag();
      if(flag & STROBE_FLAG_GOT_STROBE) {
        break;
      }

      strobe_pulse_ack();
      uart_send('.');

      // timeout - no started...
      count++;
      if(count == max) {
        res = SEND_NOT_STARTED;
        goto end_upload;
      }

      // toggle led
      led = !led;
      led_set(led);

      t0 = timer_millis();
    }
    // keep wd happy
    system_wdt_reset();
  }

  led_on();
  uart_send('{');

  // wait for transfer end
  t0 = timer_millis();
  count = 0;
  u08 busy_count = 0;
  while((flag & STROBE_FLAG_ALL_SENT)==0) {
    if(timer_millis_timed_out(t0, interval)) {
      flag = strobe_read_flag();

      // check busy
      if(flag & STROBE_FLAG_IS_BUSY) {
        uart_send('-');
        busy_count++;
      } else {
        uart_send('#');
        busy_count=0;
      }

      // type command was aborted with Ctrl-C. we are done
      if(busy_count==5) {
        res = SEND_ABORTED;
        goto end_upload;
      }

      // check timeout
      count++;
      if(count == max) {
        res = SEND_ABORTED;
        goto end_upload;
      }

      t0 = timer_millis();
    }
    // keep wd happy
    system_wdt_reset();
  }

  uart_send('}');
  led_off();

  // wait for transfer termination on Amiga side
  t0 = timer_millis();
  count = 0;
  interval = 1000;
  busy_count = 0;
  led = 0;
  while(1) {
    if(timer_millis_timed_out(t0, interval)) {
      // a filled buffer causes a delayed ack
      if(flag & STROBE_FLAG_BUFFER_FILLED) {
        uart_send('!');
        strobe_pulse_ack();
      }

      if(flag & STROBE_FLAG_IS_BUSY) {
        uart_send('-');
        busy_count++;
      } else {
        uart_send('.');
        busy_count=0;
      }

      // type command was aborted with Ctrl-C. we are done
      if(busy_count==5) {
        break;
      }

      // check timeout
      count++;
      if(count == 60) {
        res = SEND_NOT_FINISHED;
        break;
      }

      // toggle led
      led = !led;
      led_set(led);

      // next timer round
      t0 = timer_millis();

      // new flag for next timer trigger
      flag = strobe_read_flag();
    }
    // keep wd happy
    system_wdt_reset();
  }

end_upload:
  strobe_send_end();
  uart_send_pstring(PSTR("done: "));
  uart_send_hex_byte(res);
  uart_send_crlf();

  return res;
}

void knok_main(void)
{
  uart_send_pstring(PSTR("knok:"));
  strobe_init();
  led_init();
  led_on();

  timer_ms_t t0 = timer_millis();
  u08 stay = 1;
  u08 led = 1;
  u16 led_interval = 100;
  while(stay) {
    // got a strobe key?
    u32 key;
    if(strobe_get_key(&key)) {
      DL(key); DNL;
      switch(key) {
        // upload raw bootstrap code without header (for wb1.3 type)
        case KNOK_KEY_BOOT:
          knok_upload((rom_pchar)(bootstrap_code + BOOTSTRAP_HEADER_SIZE),
                      BOOTSTRAP_DATA_SIZE, PSTR("boot"));
          break;
        // upload stage1: rexx boot script
        case KNOK_KEY_REXX:
          knok_upload((rom_pchar)bootrexx_code, sizeof(bootrexx_code),
                      PSTR("rexx"));
          break;
        // upload stage2: bootstrap binary with header
        case KNOK_KEY_RXBT:
          knok_upload((rom_pchar)bootstrap_code, sizeof(bootstrap_code),
                      PSTR("RXBT"));
          break;
        // driver wants to enter device
        case STROBE_KEY_EXIT:
          stay = 0;
          break;
      }
    }

    // blink led per second
    if(timer_millis_timed_out(t0, led_interval)) {
      t0 = timer_millis();
      led = !led;
      led_set(led);
      led_interval = led ? 2000 : 100;
    }

    // keep wd happy
    system_wdt_reset();
  }

  strobe_exit();
  led_exit();

  uart_send_pstring(PSTR("boot!"));
  uart_send_crlf();
}
