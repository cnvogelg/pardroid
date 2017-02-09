#include "types.h"
#include "proto_low.h"
#include "proto.h"
#include "debug.h"

static u08 test_data[2];

void proto_init(void)
{
  proto_low_init();
}

void proto_handle(void)
{
  u08 cmd = proto_low_get_cmd();
  switch(cmd) {
    case CMD_IDLE:
      // nothing to do for now. return
      break;

    case CMD_PING:
      // alive ping from master
      DS("ping"); DNL;
      proto_low_ping();
      break;

    case CMD_TEST_READ:
      // master wants to read 2 bytes
      DS("tr:"); DB(test_data[0]); DB(test_data[1]);
      proto_low_test_read(test_data);
      DNL;
      break;

    case CMD_TEST_WRITE:
      // master wants to write 2 bytes
      DS("tw:");
      proto_low_test_write(test_data);
      DB(test_data[0]); DB(test_data[1]); DNL;
      break;

    case CMD_INVALID:
      //DS("invalid"); DNL;
      break;

    default:
      DS("?:"); DB(cmd); DNL;
      break;
  }
}
