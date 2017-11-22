#include "autoconf.h"
#include "types.h"

#define DEBUG CONFIG_DEBUG_KNOK

#include "debug.h"

#include "knok.h"
#include "strobe.h"
#include "uartutil.h"
#include "system.h"

void knok_main(void)
{
  uart_send_pstring(PSTR("knok:"));
  strobe_init();

  while(1) {
    // got a strobe key?
    u32 key;
    if(strobe_get_key(&key)) {
      DL(key); DNL;
      if(key == KNOK_KEY_BOOT) {
        break;
      }
      if(key == KNOK_KEY_EXIT) {
        break;
      }
    }
    // keep wd happy
    system_wdt_reset();
  }

  strobe_exit();
  uart_send_pstring(PSTR("boot!"));
  uart_send_crlf();
}
