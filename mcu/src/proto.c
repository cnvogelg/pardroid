#include "types.h"
#include "proto_low.h"
#include "proto.h"

void proto_init(void)
{
  proto_low_init();
}

void proto_handle(void)
{
  u08 cmd = proto_low_get_cmd();
  switch(cmd) {
    case CMD_PING:
      proto_low_ping();
      break;
    default:
      break;
  }
}
