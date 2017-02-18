#include "types.h"
#include "proto_low.h"
#include "proto.h"
#include "debug.h"
#include "mach.h"

static u16 test_data;

void proto_init(void)
{
  proto_low_init();
  test_data = 0x4812;
}

static void reg_write(u08 reg)
{
  // master wants to write a u16
  DS("rw:"); DB(reg); DC('=');
  test_data = proto_low_reg_write();
  DW(test_data); DC('.'); DNL;
}

static void reg_read(u08 reg)
{
  // master wants to reead a u16
  DS("tr:"); DB(reg); DC('='); DW(test_data);
  proto_low_reg_read(test_data);
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
      proto_low_ping();
      break;

    case CMD_RESET:
      DS("reset"); DNL;
      proto_low_ping();
      DS("restart"); DNL;
      mach_sys_reset();
      break;

    default:
      // register write?
      if((cmd >= CMD_REG_WRITE_BASE) && (cmd <= CMD_REG_WRITE_LAST)) {
        reg_write(cmd - CMD_REG_WRITE_BASE);
      }
      // register read?
      else if((cmd >= CMD_REG_READ_BASE) && (cmd <= CMD_REG_READ_LAST)) {
        reg_read(cmd - CMD_REG_READ_BASE);
      }
      // unknown?
      else {
        DS("?:"); DB(cmd); DNL;
      }
      break;
  }
}
