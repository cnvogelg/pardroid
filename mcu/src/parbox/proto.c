#include "types.h"
#include "proto_low.h"
#include "proto.h"
#include "debug.h"
#include "mach.h"

#define MAX_TEST_MSG_SIZE 8

static u16 test_data;
static u16 test_size;
static u08 test_msg[MAX_TEST_MSG_SIZE];

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
  DS("rr:"); DB(reg); DC('='); DW(test_data);
  proto_low_reg_read(test_data);
  DC('.'); DNL;
}

static void const_read(u08 reg)
{
  // master wants to reead a u16
  DS("cr:"); DB(reg); DC('='); DW(test_data);
  proto_low_reg_read(test_data);
  DC('.'); DNL;
}

static void msg_read(u08 chan)
{
  DS("mr:"); DB(chan); DC('='); DW(test_size);
  proto_low_msg_read(test_size, test_msg);
  DC('.'); DNL;
}

static void msg_write(u08 chan)
{
  DS("mw:"); DB(chan); DC('=');
  test_size = proto_low_msg_write(test_size, test_msg);
  DW(test_size); DC('.'); DNL;
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
