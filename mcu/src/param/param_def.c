#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "param.h"
#include "param_def.h"

// ----- default values -----
PARAM_DEF_VALUE_BLOCK(mac_addr, 6) =
    { 0x1a,0x11,0xaf,0xa0,0x47,0x11};
PARAM_DEF_VALUE_BLOCK(ip_addr, 4) =
    { 192, 168, 2, 42 };
PARAM_DEF_VALUE_BLOCK(net_mask, 4) =
    { 255, 255, 255, 0 };
PARAM_DEF_VALUE_BLOCK(ip_gw, 4) =
    { 192, 168, 2, 1 };
PARAM_DEF_VALUE_BYTE(boot_blk_dev, 0)

// ----- parameters -----
PARAM_DEF(PARAM_TYPE_MAC_ADDR,  0, 6, mac_addr)
PARAM_DEF(PARAM_TYPE_IP_ADDR,   6, 4, ip_addr)
PARAM_DEF(PARAM_TYPE_IP_ADDR,  10, 4, net_mask)
PARAM_DEF(PARAM_TYPE_IP_ADDR,  14, 4, ip_gw)
PARAM_DEF(PARAM_TYPE_BYTE,     18, 1, boot_blk_dev)
PARAM_DEF_TOTAL(19)
PARAM_VERSION(1)

// ----- functions -----
PARAM_FUNC_BLOCK(mac_addr, 6)
PARAM_FUNC_BLOCK(ip_addr, 4)
PARAM_FUNC_BLOCK(net_mask, 4)
PARAM_FUNC_BLOCK(ip_gw, 4)
PARAM_FUNC_BYTE(boot_blk_dev)

// ----- table -----
PARAM_TABLE_BEGIN
PARAM_TABLE_ENTRY(mac_addr),
PARAM_TABLE_ENTRY(ip_addr),
PARAM_TABLE_ENTRY(net_mask),
PARAM_TABLE_ENTRY(ip_gw),
PARAM_TABLE_ENTRY(boot_blk_dev)
PARAM_TABLE_END
