#include "types.h"
#include "arch.h"
#include "debug.h"
#include "system.h"

#include "proto_low.h"
#include "action.h"

void action_ping(void)
{
  DS("ping"); DNL;
}

void action_bootloader(void)
{
  /* do a sys reset but do not reply to command of master
     this will be done by bootloader itself */
  DS("bootloader"); DNL;
  system_sys_reset();
}

void action_reset(void)
{
  DS("reset"); DNL;
  system_sys_reset();
}

void action_handle(u08 num)
{
  u08 max = read_rom_char(&action_table_size);
  if(num >= max) {
    DS("??"); DNL;
    return;
  } else {
    u08 flags = read_rom_char(&action_table[num].flags);
    if((flags & ACTION_FLAG_NO_REPLY) == 0) {
      proto_low_action();
    }
    rom_pchar ptr = read_rom_rom_ptr(&action_table[num].func);
    action_func_t func = (action_func_t)ptr;
    func();
  }
}

void proto_api_action(u08 cmd) __attribute__ ((weak, alias("action_handle")));
