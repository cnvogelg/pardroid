#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "param.h"
#include "param_def.h"

// ----- default values -----
PARAM_DEF_VALUE_BLOCK(net_mac_addr, 6) =
    { 0x1a,0x11,0xaf,0xa0,0x47,0x11 };
PARAM_DEF_VALUE_BLOCK(net_ip_addr, 4) =
    { 192, 168, 2, 42 };
PARAM_DEF_VALUE_BLOCK(net_ip_mask, 4) =
    { 255, 255, 255, 0 };
PARAM_DEF_VALUE_BLOCK(net_ip_gw, 4) =
    { 192, 168, 2, 1 };
PARAM_DEF_VALUE_BYTE(net_dev, 0)
PARAM_DEF_VALUE_BYTE(blk_dev, 0)
PARAM_DEF_VALUE_WORD(net_mtu, 1536)

// ----- parameters -----
PARAM_DEF(PARAM_TYPE_MAC_ADDR,  0, 6, net_mac_addr)
PARAM_DEF(PARAM_TYPE_IP_ADDR,   6, 4, net_ip_addr)
PARAM_DEF(PARAM_TYPE_IP_ADDR,  10, 4, net_ip_mask)
PARAM_DEF(PARAM_TYPE_IP_ADDR,  14, 4, net_ip_gw)
PARAM_DEF(PARAM_TYPE_BYTE,     18, 1, net_dev)
PARAM_DEF(PARAM_TYPE_BYTE,     19, 1, blk_dev)
PARAM_DEF(PARAM_TYPE_WORD,     20, 2, net_mtu)
PARAM_DEF_TOTAL(22)
PARAM_VERSION(1)

// ----- functions -----
PARAM_FUNC_BLOCK(net_mac_addr, 6)
PARAM_FUNC_BLOCK(net_ip_addr, 4)
PARAM_FUNC_BLOCK(net_ip_mask, 4)
PARAM_FUNC_BLOCK(net_ip_gw, 4)
PARAM_FUNC_BYTE(net_dev)
PARAM_FUNC_BYTE(blk_dev)
PARAM_FUNC_WORD(net_mtu)

// ----- table -----
PARAM_TABLE_BEGIN
PARAM_TABLE_ENTRY(net_mac_addr),
PARAM_TABLE_ENTRY(net_ip_addr),
PARAM_TABLE_ENTRY(net_ip_mask),
PARAM_TABLE_ENTRY(net_ip_gw),
PARAM_TABLE_ENTRY(net_dev),
PARAM_TABLE_ENTRY(blk_dev),
PARAM_TABLE_ENTRY(net_mtu)
PARAM_TABLE_END
