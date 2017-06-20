#include "types.h"
#include "arch.h"
#include "debug.h"
#include "system.h"

#include "proto_low.h"
#include "action.h"
#include "status.h"

void action_nop(void)
{
}

void action_ping(void)
{
  DS("a:p"); DNL;
}

void action_bootloader(void)
{
  /* do a sys reset but do not reply to command of master
     this will be done by bootloader itself */
  DS("a:bl"); DNL;
  system_sys_reset();
}

void action_reset(void)
{
  DS("a:r"); DNL;
  system_sys_reset();
}

void action_attach(void)
{
  DS("a:a"); DNL;
  status_attach();
}

void action_detach(void)
{
  DS("a:d"); DNL;
  status_detach();
}

void action_handle(u08 num)
{
  u08 max = read_rom_char(&action_table_size);
  if(num >= max) {
    DS("a:??"); DNL;
    // wait for invalid action to time out
    proto_low_wait_cflg_hi();
    return;
  } else {
    u08 flags = read_rom_char(&action_table[num].flags);

    // handle action protocol
    u08 end = 0;
    if((flags & ACTION_FLAG_NO_REPLY) == 0) {
      proto_low_action();
      end = 1;
    }

    // end protocol before execution action
    if((flags & ACTION_FLAG_END_BEFORE) == ACTION_FLAG_END_BEFORE) {
      proto_low_end();
      end = 0;
    }

    // trigger action func
    rom_pchar ptr = read_rom_rom_ptr(&action_table[num].func);
    action_func_t func = (action_func_t)ptr;
    func();

    // restore state
    action_api_done();

    // finish action after executing function
    if(end) {
      proto_low_end();
    }
  }
}

void proto_api_action(u08 cmd) __attribute__ ((weak, alias("action_handle")));
