#include "types.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_PROTO_BOOT

#include "arch.h"
#include "debug.h"
#include "system.h"
#include "timer.h"
#include "uart.h"

#include "proto_atom.h"
#include "proto_boot.h"
#include "proto_dev.h"
#include "proto_dev_shared.h"
#include "proto_boot_shared.h"

int proto_boot_init(void)
{
  u08 cmd;

  proto_atom_init();

  while(1) {
    // get current command
    cmd = proto_atom_get_cmd();

    // boot loader!
    if(cmd == PROTO_DEV_CMD_ACTION_BOOTLOADER) {
      break;
    }

    // something else -> try to launch app
    if(cmd != PROTO_NO_CMD) {
      // return to signal that the bootloader needs
      // to enter application code
      return PROTO_BOOT_INIT_APP;
    }

    // wait for some new state
    uart_send('.');
    timer_delay(200);
    system_wdt_reset();
  }

  // handle BOOTLOADER action
  // and then enter main loop
  DC('{'); DB(cmd);
  proto_atom_action();
  DC('}');

  return PROTO_BOOT_INIT_PROTO;
}

void proto_boot_handle_cmd()
{
  // get command/channel and split it
  u08 cmd = proto_dev_get_cmd();
  if(cmd == PROTO_NO_CMD) {
    return;
  }

  switch(cmd) {
    case PROTO_BOOT_CMD_RWORD_PAGE_SIZE:
      proto_atom_read_word(proto_boot_api_get_page_size());
      break;
    case PROTO_BOOT_CMD_RLONG_ROM_SIZE:
      proto_atom_read_long(proto_boot_api_get_rom_size());
      break;
    case PROTO_BOOT_CMD_RWORD_ROM_CRC:
      proto_atom_read_word(proto_boot_api_get_rom_crc());
      break;
    case PROTO_BOOT_CMD_RWORD_ROM_MACH_TAG:
      proto_atom_read_word(proto_boot_api_get_rom_mach_tag());
      break;
    case PROTO_BOOT_CMD_RWORD_ROM_FW_VERSION:
      proto_atom_read_word(proto_boot_api_get_rom_fw_version());
      break;
    case PROTO_BOOT_CMD_RWORD_ROM_FW_ID:
      proto_atom_read_word(proto_boot_api_get_rom_fw_id());
      break;
    case PROTO_BOOT_CMD_WLONG_PAGE_ADDR:
      {
        u32 addr = proto_atom_write_long();
        proto_boot_api_set_page_addr(addr);
        break;
      }
    case PROTO_BOOT_CMD_WBLOCK_PAGE_WRITE:
      {
        u08 *buf;
        u16 size;
        proto_boot_api_get_page_write_buf(&buf, &size);
        proto_atom_write_block_nospi(buf, size);
        proto_boot_api_flash_page();
        break;
      }
    case PROTO_BOOT_CMD_RBLOCK_PAGE_READ:
      {
        u08 *buf;
        u16 size;
        proto_boot_api_get_page_read_buf(&buf, &size);
        proto_atom_read_block_nospi(buf, size);
        break;
      }
    default:
      DC('!'); DC('O'); DB(cmd); DNL;
      break;
  }
}