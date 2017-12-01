#ifndef WIZNET_H
#define WIZNET_H

extern void wiznet_init(void);

extern void wiznet_set_mac(const u08 mac[6]);
extern void wiznet_set_src_addr(const u08 addr[4]);
extern void wiznet_set_net_mask(const u08 mask[4]);
extern void wiznet_set_gw_addr(const u08 addr[4]);

extern u08  wiznet_is_link_up(void);

#endif
