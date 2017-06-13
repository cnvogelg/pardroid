#include "types.h"
#include "arch.h"
#include "debug.h"
#include "system.h"

#include "proto_low.h"
#include "proto_shared.h"
#include "func.h"

static u16 regaddr;

static void regaddr_set(void)
{
  DS("ras:");
  regaddr = proto_low_write_word();
  DW(regaddr); DC('.'); DNL;
}

static void regaddr_get(void)
{
  DS("rag:"); DW(regaddr);
  proto_low_read_word(regaddr);
  DC('.'); DNL;
}

static void reg_write(void)
{
  // master wants to write a u16
  DS("rw:");
  u16 val = proto_low_write_word();
  DW(regaddr); DC('='); DW(val);
  func_api_set_reg(regaddr & 0xff, val);
  DC('.'); DNL;
}

static void reg_read(void)
{
  // master wants to reead a u16
  DS("rr:"); DW(regaddr); DC('=');
  u16 val = func_api_get_reg(regaddr & 0xff);
  DW(val);
  proto_low_read_word(val);
  DC('.'); DNL;
}

void func_handle(u08 num)
{
  DS("f:"); DB(num); DNL;
  switch(num) {
    case PROTO_FUNC_REGADDR_GET:
      regaddr_get();
      break;
    case PROTO_FUNC_REGADDR_SET:
      regaddr_set();
      break;
    case PROTO_FUNC_REG_WRITE:
      reg_write();
      break;
    case PROTO_FUNC_REG_READ:
      reg_read();
      break;
    default:
      DS("???"); DNL;
      break;
  }
}

void proto_api_function(u08 num) __attribute__ ((weak, alias("func_handle")));
