#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG CONFIG_DEBUG_KNOK

#include "debug.h"

#include "knok.h"
#include "strobe.h"
#include "uartutil.h"
#include "hw_system.h"
#include "hw_timer.h"
#include "hw_led.h"
#include "rominfo.h"
#include "proto/wire_dev.h"

#define SEND_OK                  0
#define SEND_NOT_STARTED         1
#define SEND_ABORTED             2
#define SEND_NOT_FINISHED        3

#define BIND(x) __attribute__ ((weak, alias(#x)))

// default handler for upload
// re-implement these in your own code!
static u08 knok_upload_byte_dummy(void)
{
  return '?';
}

static knok_api_upload_byte_func_t knok_upload_dummy(u16 *size)
{
  *size = 3;
  return knok_upload_byte_dummy;
}

knok_api_upload_byte_func_t knok_api_upload_boot(u16 *) BIND(knok_upload_dummy);
knok_api_upload_byte_func_t knok_api_upload_rexx(u16 *) BIND(knok_upload_dummy);
knok_api_upload_byte_func_t knok_api_upload_rxbt(u16 *) BIND(knok_upload_dummy);

static rom_pchar  rom_data_ptr;

static u08 knok_upload_byte_rom(void)
{
  u08 data = read_rom_char(rom_data_ptr);
  rom_data_ptr++;
  return data;
}


static u08 knok_upload(u16 size, knok_api_upload_byte_func_t byte_func, rom_pchar title)
{
  u08 res = SEND_OK;

  uart_send_pstring(PSTR("upload:+"));
  uart_send_hex_word(size);
  uart_send(':');
  uart_send_pstring(title);

  // nothing to do?
  if(size == 0) {
    return SEND_NOT_STARTED;
  }

  strobe_send_begin(byte_func, size);

  u16 interval = 200;
  u08 flag = STROBE_FLAG_NONE;
  const u08 max = 100;

  // wait for first strobe
  strobe_pulse_ack();
  uart_send('.');

  hw_timer_ms_t t0 = hw_timer_millis();
  u08 count = 0;
  u08 led = 1;
  while(1) {
    if(hw_timer_millis_timed_out(t0, interval)) {
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
      hw_led_set(led);

      t0 = hw_timer_millis();
    }
  }

  hw_led_on();
  uart_send('{');

  // wait for transfer end
  t0 = hw_timer_millis();
  count = 0;
  interval = 1000;
  u08 busy_count = 0;
  while((flag & STROBE_FLAG_ALL_SENT)==0) {
    if(hw_timer_millis_timed_out(t0, interval)) {
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

      t0 = hw_timer_millis();
    }
  }

  uart_send('}');
  hw_led_off();

  // wait for transfer termination on Amiga side
  t0 = hw_timer_millis();
  count = 0;
  busy_count = 0;
  led = 0;
  while(1) {
    if(hw_timer_millis_timed_out(t0, interval)) {
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
      hw_led_set(led);

      // next timer round
      t0 = hw_timer_millis();

      // new flag for next timer trigger
      flag = strobe_read_flag();
    }
  }

end_upload:
  strobe_send_end();
  uart_send_pstring(PSTR("done: "));
  uart_send_hex_byte(res);
  uart_send_crlf();

  return res;
}

void blink_hello(void)
{
  uart_send_pstring(PSTR("helo!"));
  for(u08 i=0;i<5;i++) {
    hw_led_on();
    hw_timer_delay_ms(100);
    hw_led_off();
    hw_timer_delay_ms(100);
  }
}

void knok_main(void)
{
  uart_send_pstring(PSTR("knok:"));

  // only setup port so we can read data
  strobe_init_port();

  // --- early exit ---
  // is a reset command active? then directly exit to proto
  // we also accept bootloader to enter main loop if no bootloader is available
  // otherwise we would constantly stay in a reset loop
  u08 data = strobe_get_data();
  if(data == PROTO_DEV_CMD_ACTION_RESET || data == PROTO_DEV_CMD_ACTION_BOOTLOADER) {
    uart_send_pstring(PSTR("exyt"));
    uart_send_crlf();
    return;
  }

  // we will entry knok mode so enable irq for strobe
  strobe_init_irq();
  hw_led_init();
  hw_led_on();

  hw_timer_ms_t t0 = hw_timer_millis();
  u08 led = 1;
  u16 led_interval = 100;
  while(1) {

    // if a reset or bootloader command is active then reset immediately
    // bootloader command will be answered by bootloader
    // RESET_ACTION leaves knok mode in early exit next time (see above)
    u08 data = strobe_get_data();
    if(data == PROTO_DEV_CMD_ACTION_RESET || data == PROTO_DEV_CMD_ACTION_BOOTLOADER) {
      uart_send_pstring(PSTR("bail"));
      uart_send_crlf();
      hw_system_reset();
      break;
    }

    // got a strobe key?
    u32 key;
    if(strobe_get_key(&key)) {
      DL(key); DNL;
      u16 size;
      knok_api_upload_byte_func_t byte_func;
      switch(key) {
        // upload raw bootstrap code without header (for wb1.3 type)
        case KNOK_KEY_BOOT:
          byte_func = knok_api_upload_boot(&size);
          knok_upload(size, byte_func, PSTR("boot"));
          break;
        // upload stage1: rexx boot script
        case KNOK_KEY_REXX:
          byte_func = knok_api_upload_rexx(&size);
          knok_upload(size, byte_func, PSTR("rexx"));
          break;
        // upload stage2: bootstrap binary with header
        case KNOK_KEY_RXBT:
          byte_func = knok_api_upload_rxbt(&size);
          knok_upload(size, byte_func, PSTR("rxbt"));
          break;
        // show rom info
        case KNOK_KEY_INFO:
          rom_data_ptr = rom_info_str;
          knok_upload(ROMINFO_SIZE, knok_upload_byte_rom, PSTR("info"));
          break;
        // blink hello
        case KNOK_KEY_HELO:
          blink_hello();
          break;
      }
    }

    // blink led per second
    if(hw_timer_millis_timed_out(t0, led_interval)) {
      t0 = hw_timer_millis();
      led = !led;
      hw_led_set(led);
      led_interval = led ? 2000 : 100;
    }
  }

  strobe_exit();
  hw_led_exit();

  uart_send_pstring(PSTR("boot!"));
  uart_send_crlf();
}
