#ifndef WIZNET_H
#define WIZNET_H

#define WIZNET_NO_SOCKET        0xff

#define WIZNET_RESULT_OK            0
#define WIZNET_RESULT_CMD_TIMEOUT   1
#define WIZNET_RESULT_SEND_TIMEOUT  2

extern void wiznet_init(void);

extern void wiznet_set_mac(const u08 mac[6]);
extern void wiznet_set_src_addr(const u08 addr[4]);
extern void wiznet_set_net_mask(const u08 mask[4]);
extern void wiznet_set_gw_addr(const u08 addr[4]);

extern void wiznet_get_mac(u08 mac[6]);
extern void wiznet_get_src_addr(u08 addr[4]);
extern void wiznet_get_net_mask(u08 mask[4]);
extern void wiznet_get_gw_addr(u08 addr[4]);

extern u08  wiznet_is_link_up(void);
extern u08  wiznet_find_free_socket(void);

extern u08  wiznet_udp_open(u08 sock, u16 my_port);
extern u08  wiznet_udp_close(u08 sock);
extern u08  wiznet_udp_send(u08 sock, const u08 *buf, u16 len,
                            const u08 addr[4], u16 port);
extern u08  wiznet_udp_is_recv_pending(u08 sock);
extern u16  wiznet_udp_recv(u08 sock, u08 *buf, u16 max_len,
                            u08 addr[4], u16 *port);

#endif
