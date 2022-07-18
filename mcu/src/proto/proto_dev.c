#include "types.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_PROTO_DEV

#include "arch.h"
#include "debug.h"
#include "hw_system.h"
#include "hw_timer.h"

#include "proto_atom.h"
#include "proto_dev.h"
#include "proto/wire_dev.h"
#include "knok.h"
#include "fw_info.h"

static u16 driver_token;

void proto_dev_init()
{
  // before setting up proto we try to reach knok mode
  // if knok mode is entered then stay in knok_main() and
  // only return if a RESET or BOOTLOADER action is detected
  //
  // BOOTLOADER only occurs if no bootloader was found
  // then gracefully handle it like a reset
  DC('['); DNL;
  knok_main();
  DC(']'); DNL;

  // init proto now
  proto_atom_init();

  // just to be sure: expect reset/bootloader command
  u08 cmd = proto_atom_get_cmd();
  if(cmd != PROTO_DEV_CMD_ACTION_RESET && cmd != PROTO_DEV_CMD_ACTION_BOOTLOADER) {
    DC('?'); DB(cmd); DNL;
    hw_system_sys_reset();
  }

  // handle RESET or BOOTLOADER action
  // and then enter main loop
  DC('{'); DB(cmd);
  proto_atom_action();
  DC('}');
}

static void proto_dev_handle(u08 cmd)
{
  switch(cmd) {
    /* actions */
    case PROTO_DEV_CMD_ACTION_PING:
      DC('!'); DC('P');
      proto_atom_action();
      break;
    case PROTO_DEV_CMD_ACTION_BOOTLOADER:
    case PROTO_DEV_CMD_ACTION_RESET:
      // to perform reset or enter bootloader:
      // immediate reset to reach proto_dev_init()/boot code again
      // and ack the action there
      DC('!'); DB(cmd); DNL;
      hw_timer_delay_ms(10);
      hw_system_sys_reset();
      break;
    case PROTO_DEV_CMD_ACTION_KNOK:
      // to enter knok mode: finish action and then resets
      DC('!'); DC('K'); DNL;
      proto_atom_action();
      hw_timer_delay_ms(10);
      hw_system_sys_reset();
      break;
    /* device constants */
    case PROTO_DEV_CMD_RWORD_FW_ID:
      DC('*'); DC('I');
      proto_atom_read_word(FW_GET_ID());
      break;
    case PROTO_DEV_CMD_RWORD_FW_VERSION:
      DC('*'); DC('V');
      proto_atom_read_word(FW_GET_VERSION());
      break;
    case PROTO_DEV_CMD_RWORD_MACH_TAG:
      DC('*'); DC('T');
      proto_atom_read_word(FW_GET_MACHTAG());
      break;
    case PROTO_DEV_CMD_RWORD_DRIVER_TOKEN:
      DC('*'); DC('D'); DW(driver_token);
      proto_atom_read_word(driver_token);
      break;
    /* writes */
    case PROTO_DEV_CMD_WWORD_DRIVER_TOKEN:
      DC('#');
      driver_token = proto_atom_write_word();
      DW(driver_token);
      break;
    /* unknown driver command */
    default:
      DC('!'); DC('?');
      break;
  }
}

u08 proto_dev_get_cmd(void)
{
  u08 cmd = proto_atom_get_cmd();
  // is driver command?
  if((cmd & PROTO_CMD_MASK) == PROTO_DEV_CMD_MASK) {
    proto_dev_handle(cmd);
    // already handled...
    return PROTO_NO_CMD;
  }
  return cmd;
}
