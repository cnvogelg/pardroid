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
      case PROTO_REG_GLOBAL_MAGIC:
        return PROTO_MAGIC_VALUE;
      case PROTO_REG_GLOBAL_ID:
        return pablo_get_rom_fw_id();
      case PROTO_REG_GLOBAL_MACHTAG:
        return pablo_get_mach_tag();
      case PROTO_REG_GLOBAL_VERSION:
        return pablo_get_rom_version();
    }
  }
  return reg_api_get_value(reg_range, reg_num);
}

u16  reg_wfunc_read_handle(u08 num)
{
  if(num == PROTO_WFUNC_REG_ADDR) {
    return reg_get_addr();
  }
  else if(num == PROTO_WFUNC_REG_VALUE) {
    return reg_get_value();
  }
  return 0;
}

void reg_wfunc_write_handle(u08 num, u16 val)
{
  if(num == PROTO_WFUNC_REG_ADDR) {
    reg_set_addr(val);
  }
  else if(num == PROTO_WFUNC_REG_VALUE) {
    reg_set_value(val);
  }
}

u16  proto_api_wfunc_read(u08 chn) __attribute__ ((weak, alias("reg_wfunc_read_handle")));
void proto_api_wfunc_write(u08 chn, u16 val) __attribute__ ((weak, alias("reg_wfunc_write_handle")));
