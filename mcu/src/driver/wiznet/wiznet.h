#ifndef WIZNET_H
#define WIZNET_H

#define WIZNET_NO_SOCKET        0xff

#define WIZNET_RESULT_OK            0
#define WIZNET_RESULT_CMD_TIMEOUT   1
#define WIZNET_RESULT_SEND_TIMEOUT  2
#define WIZNET_RESULT_STATE_TIMEOUT 3
#define WIZNET_RESULT_SIZE_TIMEOUT  4
#define WIZNET_RESULT_NO_DATA       5
#define WIZNET_RESULT_NO_MEMORY     6

typedef struct {
  u08  addr[4];
  u16  port;
  u16  len;
} wiz_udp_pkt_t;

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

// ----- UDP -----
extern u08  wiznet_udp_open(u08 sock, u16 my_port);
extern u08  wiznet_udp_close(u08 sock);

extern u08  wiznet_udp_send(u08 sock, const u08 *buf, const wiz_udp_pkt_t *pkt);
extern u08  wiznet_udp_send_begin(u08 sock, const wiz_udp_pkt_t *pkt);
extern void wiznet_udp_send_data(u08 sock, u16 offset, const u08 *buf, u16 len);
extern u08  wiznet_udp_send_end(u08 sock, const wiz_udp_pkt_t *pkt);

extern u08  wiznet_udp_is_recv_pending(u08 sock, wiz_udp_pkt_t *pkt);
extern u08  wiznet_udp_recv(u08 sock, u08 *buf, u16 max_len, wiz_udp_pkt_t *pkt);
extern u08  wiznet_udp_recv_begin(u08 sock, wiz_udp_pkt_t *pkt);
extern void wiznet_udp_recv_data(u08 sock, u16 offset, u08 *buf, u16 len);
extern u08  wiznet_udp_recv_end(u08 sock, const wiz_udp_pkt_t *pkt);

// ----- ETH Frames (Socket #0) -----
extern u08  wiznet_eth_open(u08 filter);
extern u08  wiznet_eth_close(void);

extern u08  wiznet_eth_send(const u08 *buf, u16 len);
extern u08  wiznet_eth_send_begin(u16 len);
extern void wiznet_eth_send_data(u16 offset, const u08 *buf, u16 len);
extern u08  wiznet_eth_send_end(u16 len);

extern u08  wiznet_eth_is_recv_pending(void);
extern u08  wiznet_eth_recv(u08 *buf, u16 max_len, u16 *len);
extern u08  wiznet_eth_recv_begin(u16 *len);
extern void wiznet_eth_recv_data(u16 offset, u08 *buf, u16 len);
extern u08  wiznet_eth_recv_end(u16 len);

#endif
