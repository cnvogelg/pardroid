#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_ACTION

#include "debug.h"
#include "system.h"

#include "proto_low.h"
#include "action.h"
#include "status.h"
#include "proto.h"

void action_ping(void)
{
  DC('p'); DNL;
}

void action_bootloader(void)
{
  /* do a sys reset but do not reply to command of master
     this will be done by bootloader itself */
  DC('b'); DNL;
  system_sys_reset();
}

void action_reset(void)
{
  DC('r'); DNL;
  system_sys_reset();
}

void action_attach(void)
{
  DC('a'); DNL;
  status_attach();
}

void action_detach(void)
{
  DC('d'); DNL;
  status_detach();
}

void action_handle(u08 num)
{
  u08 max = read_rom_char(&action_table_size);
  if(num >= max) {
    DC('?'); DNL;
    // wait for invalid action to time out
    action_ping();
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
      u08 status = proto_api_get_end_status();
      proto_low_end(status);
      end = 0;
    }

    // trigger action func
    action_func_t func = (action_func_t)read_rom_rom_ptr(&action_table[num].func);
    func();

    // finish action after executing function
    if(end) {
      u08 status = proto_api_get_end_status();
      proto_low_end(status);
      DS("as:"); DB(status); DNL;
    }
  }
}

void proto_api_action(u08 cmd) __attribute__ ((weak, alias("action_handle")));
