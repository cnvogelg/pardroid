#include "types.h"
#include "proto_low.h"
#include "proto.h"
#include "debug.h"
#include "mach.h"

void proto_init(void)
{
  proto_low_init();
}

static void reg_write(u08 reg)
{
  // master wants to write a u16
  DS("rw:"); DB(reg); DC('=');
  u16 val = proto_low_write_word();
  DW(val);
  proto_api_set_reg(reg, val);
  DC('.'); DNL;
}

static void reg_read(u08 reg)
{
  // master wants to reead a u16
  DS("rr:"); DB(reg); DC('=');
  u16 val = proto_api_get_reg(reg);
  DW(val);
  proto_low_read_word(val);
  DC('.'); DNL;
}

static void const_read(u08 reg)
{
  // master wants to reead a u16
  DS("cr:"); DB(reg); DC('=');
  u16 val = proto_api_get_const(reg);
  DW(val);
  proto_low_read_word(val);
  DC('.'); DNL;
}

static void msg_read(u08 chan)
{
  DS("mr:"); DB(chan); DC('=');
  u16 size = 0;
  u08 *buf = proto_api_get_read_msg(&size);
  DW(size);
  proto_low_read_block(size, buf);
  DC('.'); DNL;
}

static void msg_write(u08 chan)
{
  DS("mw:"); DB(chan); DC('=');
  u16 max_size = 0;
  u08 *buf = proto_api_get_write_msg(&max_size);
  DW(max_size); DC(':');
  u16 size = proto_low_write_block(max_size, buf);
  DW(size);
  proto_api_set_write_msg_size(size);
  DC('.'); DNL;
}

void proto_handle(void)
{
  u08 cmd = proto_low_get_cmd();
  switch(cmd) {
    case CMD_IDLE:
      // nothing to do for now. return
      break;
    case CMD_INVALID:
      //DS("invalid"); DNL;
      break;

    case CMD_PING:
      // alive ping from master
      DS("ping"); DNL;
      proto_low_no_value();
      break;

    case CMD_BOOTLOADER:
      // immediately reset to bootloader
      // do not complete ping protocol here as it is done in bootloader
      DS("bootloader"); DNL;
      mach_sys_reset();
      break;

    case CMD_RESET:
      DS("reset"); DNL;
      proto_low_no_value();
      DS("restart"); DNL;
      mach_sys_reset();
      break;

    default:
      {
        u08 cmd_base = cmd & CMD_MASK;
        switch(cmd_base) {
          case CMD_REG_WRITE:
            reg_write(cmd - cmd_base);
            break;
          case CMD_REG_READ:
            reg_read(cmd - cmd_base);
            break;
          case CMD_MSG_WRITE:
            msg_write(cmd - cmd_base);
            break;
          case CMD_MSG_READ:
            msg_read(cmd - cmd_base);
            break;
          case CMD_CONST_READ:
            const_read(cmd - cmd_base);
            break;
          default:
            DS("?:"); DB(cmd); DNL;
            break;
        }
      }
      break;
  }
}
