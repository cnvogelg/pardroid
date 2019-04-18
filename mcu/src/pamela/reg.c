#include "types.h"
#include "arch.h"
#include "autoconf.h"

#include "proto_shared.h"
#include "pablo.h"
#include "reg.h"

static u08 reg_num;
static u08 reg_range;

void reg_set_addr(u16 addr)
{
  reg_num = (u08)(addr & 0xff);
  reg_range = (u08)(addr >> 8);
}

u16 reg_get_addr(void)
{
  return reg_range << 8 | reg_num;
}

void reg_set_value(u16 val)
{
  reg_api_set_value(reg_range, reg_num, val);
}

u16 reg_get_value(void)
{
  if(reg_range == PROTO_REG_RANGE_GLOBAL) {
    switch(reg_num) {
      case PROTO_REG_GLOBAL_FW_ID:
        return pablo_get_rom_fw_id();
      case PROTO_REG_GLOBAL_MACHTAG:
        return pablo_get_mach_tag();
      case PROTO_REG_GLOBAL_FW_VERSION:
        return pablo_get_rom_version();
    }
  }
  return reg_api_get_value(reg_range, reg_num);
}
