#include "autoconf.h"
#include "types.h"

#define DEBUG CONFIG_DEBUG_DRIVER_WIZNET

#include "debug.h"

#include "wiznet.h"
#include "wiznet_low.h"
#include "wiz_io.h"
#include "wiz_defs.h"

#include "timer.h"
#include "uartutil.h"
#include "system.h"

// configure socket count
#define NUM_SOCKETS  CONFIG_DRIVER_WIZNET_NUM_SOCKETS
#define MEM_SIZE     (32 / NUM_SOCKETS)
#define MEM_BYTES    (MEM_SIZE * 1024)

#define WIZ_CMD_TIMEOUT   100

// ----- helper functions -----

static inline void set_src_port(u08 sock, u16 port)
{
  wiz_io_socket_reg_write_word(sock, WIZ_REG_SOCKET_SRC_PORT, port);
}

static inline void set_dest_addr(u08 sock, const u08 addr[4])
{
  wiz_io_socket_reg_write_buf(sock, WIZ_REG_SOCKET_DST_IP, addr, 4);
}

static inline void set_dest_port(u08 sock, u16 port)
{
  wiz_io_socket_reg_write_word(sock, WIZ_REG_SOCKET_DST_PORT, port);
}

static inline void write_ir(u08 sock, u08 val)
{
  wiz_io_socket_reg_write(sock, WIZ_REG_SOCKET_IR, val);
}

static inline u08 read_ir(u08 sock)
{
  return wiz_io_socket_reg_read(sock, WIZ_REG_SOCKET_IR);
}

static inline void write_mode(u08 sock, u08 mode)
{
  wiz_io_socket_reg_write(sock, WIZ_REG_SOCKET_MODE, mode);
}

static inline u08 read_mode(u08 sock)
{
  return wiz_io_socket_reg_read(sock, WIZ_REG_SOCKET_MODE);
}

static inline u08 read_status(u08 sock)
{
  return wiz_io_socket_reg_read(sock, WIZ_REG_SOCKET_STATUS);
}

u08 exec_cmd(u08 sock, u08 cmd)
{
  wiz_io_socket_reg_write(sock, WIZ_REG_SOCKET_CMD, cmd);

  // CMD reg is cleared if command was accepted
  timer_ms_t t0 = timer_millis();
  while(wiz_io_socket_reg_read(sock, WIZ_REG_SOCKET_CMD)) {
    system_wdt_reset();
    if(timer_millis_timed_out(t0, WIZ_CMD_TIMEOUT)) {
      return WIZNET_RESULT_CMD_TIMEOUT;
    }
  }
  return WIZNET_RESULT_OK;
}

u08 wait_state(u08 sock, u08 state)
{
  timer_ms_t t0 = timer_millis();
  while(1) {
    system_wdt_reset();
    u08 sr = read_status(sock);
    if(sr == state) {
      break;
    }
    if(timer_millis_timed_out(t0, WIZ_CMD_TIMEOUT)) {
      return WIZNET_RESULT_STATE_TIMEOUT;
    }
  }
  return WIZNET_RESULT_OK;
}

u08 exec_cmd_state(u08 sock, u08 cmd, u08 state)
{
  u08 res = exec_cmd(sock, cmd);
  if(res != WIZNET_RESULT_OK) {
    return res;
  }
  return wait_state(sock, state);
}

// ----- API -----

void wiznet_init(void)
{
  wiznet_low_reset();
  timer_delay(200); // 150ms in data sheet WIZ820io

  wiz_io_base_reg_write(WIZ_REG_BASE_MODE, WIZ_MASK_RESET);

  // configure socket rx/tx buffer size
  u08 socket = 0;
  while(socket < NUM_SOCKETS) {
    wiz_io_socket_reg_write(socket, WIZ_REG_SOCKET_RX_MEMSIZE, MEM_SIZE);
    wiz_io_socket_reg_write(socket, WIZ_REG_SOCKET_TX_MEMSIZE, MEM_SIZE);
    socket++;
  }
  while(socket < WIZ_MAX_SOCKETS) {
    wiz_io_socket_reg_write(socket, WIZ_REG_SOCKET_RX_MEMSIZE, 0);
    wiz_io_socket_reg_write(socket, WIZ_REG_SOCKET_TX_MEMSIZE, 0);
    socket++;
  }
}

void wiznet_set_mac(const u08 mac[6])
{
  wiz_io_base_reg_write_buf(WIZ_REG_BASE_SRC_MAC, mac, 6);
}

void wiznet_set_src_addr(const u08 addr[4])
{
  wiz_io_base_reg_write_buf(WIZ_REG_BASE_SRC_ADDR, addr, 4);
}

void wiznet_set_net_mask(const u08 mask[4])
{
  wiz_io_base_reg_write_buf(WIZ_REG_BASE_NET_MASK, mask, 4);
}

void wiznet_set_gw_addr(const u08 addr[4])
{
  wiz_io_base_reg_write_buf(WIZ_REG_BASE_GW_ADDR, addr, 4);
}

void wiznet_get_mac(u08 mac[6])
{
  wiz_io_base_reg_read_buf(WIZ_REG_BASE_SRC_MAC, mac, 6);
}

void wiznet_get_src_addr(u08 addr[4])
{
  wiz_io_base_reg_read_buf(WIZ_REG_BASE_SRC_ADDR, addr, 4);
}

void wiznet_get_net_mask(u08 mask[4])
{
  wiz_io_base_reg_read_buf(WIZ_REG_BASE_NET_MASK, mask, 4);
}

void wiznet_get_gw_addr(u08 addr[4])
{
  wiz_io_base_reg_read_buf(WIZ_REG_BASE_GW_ADDR, addr, 4);
}

u08 wiznet_is_link_up(void)
{
  u08 val = wiz_io_base_reg_read(WIZ_REG_BASE_PHY_STATUS);
  return (val & WIZ_MASK_LINK_UP) == WIZ_MASK_LINK_UP;
}

u08 wiznet_find_free_socket()
{
  for(u08 socket=0;socket < NUM_SOCKETS; socket++) {
    u08 mode = read_mode(socket);
    if((mode & WIZ_SOCKET_MODE_MASK) == WIZ_SOCKET_MODE_CLOSED) {
      return socket;
    }
  }
  return WIZNET_NO_SOCKET;
}

static u08 send_cmd(u08 sock)
{
  u08 res = exec_cmd(sock, WIZ_SOCKET_CMD_SEND);
  if(res != WIZNET_RESULT_OK) {
    return res;
  }

  timer_ms_t t0 = timer_millis();
  while(1) {
    system_wdt_reset();
    if(timer_millis_timed_out(t0, WIZ_CMD_TIMEOUT)) {
      return WIZNET_RESULT_SEND_TIMEOUT;
    }
    u08 ir = read_ir(sock);
    if((ir & WIZ_SOCKET_IR_SEND_OK) == WIZ_SOCKET_IR_SEND_OK) {
      write_ir(sock, WIZ_SOCKET_IR_SEND_OK);
      return WIZNET_RESULT_OK;
    }
    if((ir & WIZ_SOCKET_IR_TIMEOUT) == WIZ_SOCKET_IR_TIMEOUT) {
      write_ir(sock, WIZ_SOCKET_IR_SEND_OK | WIZ_SOCKET_IR_TIMEOUT);
      return WIZNET_RESULT_SEND_TIMEOUT;
    }
  }
}

// ---------- UDP ----------

u08 wiznet_udp_open(u08 sock, u16 my_port)
{
  write_mode(sock, WIZ_SOCKET_MODE_UDP);
  set_src_port(sock, my_port);
  return exec_cmd_state(sock, WIZ_SOCKET_CMD_OPEN, WIZ_SOCKET_STATUS_UDP);
}

u08 wiznet_udp_close(u08 sock)
{
  u08 res = exec_cmd_state(sock, WIZ_SOCKET_CMD_CLOSE, WIZ_SOCKET_STATUS_CLOSED);
  write_ir(sock, 0xff);
  write_mode(sock, WIZ_SOCKET_MODE_CLOSED);
  return res;
}

u08 wiznet_udp_send(u08 sock, const u08 *buf, const wiz_udp_pkt_t *pkt)
{
  u16 free = wiz_io_get_tx_free(0);
  if(free < pkt->len) {
    return WIZNET_RESULT_NO_MEMORY;
  }

  set_dest_addr(sock, pkt->addr);
  set_dest_port(sock, pkt->port);

  // copy data
  wiz_io_tx_buffer_write(sock, 0, buf, pkt->len);
  wiz_io_tx_buffer_confirm(sock, pkt->len);

  return send_cmd(sock);
}

u08 wiznet_udp_send_begin(u08 sock, const wiz_udp_pkt_t *pkt)
{
  u16 free = wiz_io_get_tx_free(0);
  if(free < pkt->len) {
    return WIZNET_RESULT_NO_MEMORY;
  }
  return WIZNET_RESULT_OK;
}

void wiznet_udp_send_data(u08 sock, u16 offset, const u08 *buf, u16 len)
{
  wiz_io_tx_buffer_write(sock, offset, buf, len);
}

u08  wiznet_udp_send_end(u08 sock, const wiz_udp_pkt_t *pkt)
{
  wiz_io_tx_buffer_confirm(sock, pkt->len);

  set_dest_addr(sock, pkt->addr);
  set_dest_port(sock, pkt->port);

  return send_cmd(sock);
}

u08 wiznet_udp_is_recv_pending(u08 sock, wiz_udp_pkt_t *pkt)
{
  u16 size = wiz_io_get_rx_size(sock);
  if(size == 0) {
    if(pkt != 0) {
      pkt->len = 0;
    }
    return 0;
  }

  // fill packet info
  if(pkt != 0) {
    u08 tmp[8];
    wiz_io_rx_buffer_read(sock, 0, tmp, 8);

    // copy IP
    for(u08 i=0;i<4;i++) {
      pkt->addr[i] = tmp[i];
    }
    // copy port
    pkt->port  = tmp[4] << 8 | tmp[5];
    // set size
    pkt->len = tmp[6] << 8 | tmp[7];
  }

  return 1;
}

u08 wiznet_udp_recv(u08 sock, u08 *buf, u16 max_len,
                    wiz_udp_pkt_t *pkt)
{
  if(!wiznet_udp_is_recv_pending(sock, pkt)) {
    return WIZNET_RESULT_NO_DATA;
  }

  // packet fits into buffer
  u16 read_size = pkt->len;
  if(max_len < pkt->len) {
    read_size = max_len;
  }

  // rear packet data
  wiz_io_rx_buffer_read(sock, 8, buf, read_size);

  // confirm full buffer
  wiz_io_rx_buffer_confirm(sock, pkt->len + 8);

  // trigger next receive
  return exec_cmd(sock, WIZ_SOCKET_CMD_RECV);
}

u08  wiznet_udp_recv_begin(u08 sock, wiz_udp_pkt_t *pkt)
{
  if(!wiznet_udp_is_recv_pending(sock, pkt)) {
    return WIZNET_RESULT_NO_DATA;
  }
  return WIZNET_RESULT_OK;
}

void  wiznet_udp_recv_data(u08 sock, u16 offset, u08 *buf, u16 len)
{
  wiz_io_rx_buffer_read(sock, offset + 8, buf, len);
}

u08 wiznet_udp_recv_end(u08 sock, const wiz_udp_pkt_t *pkt)
{
  // confirm full buffer
  wiz_io_rx_buffer_confirm(sock, pkt->len + 8);

  // trigger next receive
  return exec_cmd(sock, WIZ_SOCKET_CMD_RECV);
}

// ---------- ETH ----------

u08  wiznet_eth_open(u08 filter)
{
  u08 mode = WIZ_SOCKET_MODE_MACRAW;
  if(filter) {
    mode |= WIZ_MASK_MAC_FILTER;
  }
  write_mode(0, mode);
  return exec_cmd_state(0, WIZ_SOCKET_CMD_OPEN, WIZ_SOCKET_STATUS_MACRAW);
}

u08  wiznet_eth_close(void)
{
  u08 res = exec_cmd_state(0, WIZ_SOCKET_CMD_CLOSE, WIZ_SOCKET_STATUS_CLOSED);
  write_ir(0, 0xff);
  write_mode(0, WIZ_SOCKET_MODE_CLOSED);
  return res;
}

u08  wiznet_eth_send(const u08 *buf, u16 len)
{
  u08 ok = wiznet_eth_send_begin(len);
  if(ok != WIZNET_RESULT_OK) {
    return ok;
  }

  wiz_io_tx_buffer_write(0, 0, buf, len);
  wiz_io_tx_buffer_confirm(0, len);

  return send_cmd(0);
}

u08  wiznet_eth_send_begin(u16 len)
{
  if(len > MEM_BYTES) {
    return WIZNET_RESULT_NO_MEMORY;
  }

  timer_ms_t t0 = timer_millis();
  while(1) {
    system_wdt_reset();
    if(timer_millis_timed_out(t0, WIZ_CMD_TIMEOUT)) {
      return WIZNET_RESULT_SIZE_TIMEOUT;
    }

    u16 free = wiz_io_get_tx_free(0);
    if(free >= len) {
      break;
    }
  }
  return WIZNET_RESULT_OK;
}

void wiznet_eth_send_data(u16 offset, const u08 *buf, u16 len)
{
  wiz_io_tx_buffer_write(0, offset, buf, len);
}

u08  wiznet_eth_send_end(u16 len)
{
  wiz_io_tx_buffer_confirm(0, len);

  return send_cmd(0);
}

u08  wiznet_eth_is_recv_pending(void)
{
  u16 size = wiz_io_get_rx_size(0);
  return size > 0;
}

u08  wiznet_eth_recv(u08 *buf, u16 max_len, u16 *len)
{
  wiznet_eth_recv_begin(len);

  u16 copy_size = *len;
  if(max_len < copy_size) {
    copy_size = max_len;
  }

  wiz_io_rx_buffer_read(0, 2, buf, copy_size);
  wiz_io_rx_buffer_confirm(0, *len + 2);

  // trigger next receive
  return exec_cmd(0, WIZ_SOCKET_CMD_RECV);
}

u08  wiznet_eth_recv_begin(u16 *len)
{
  // read packet size
  u08 tmp[2];
  wiz_io_rx_buffer_read(0, 0, tmp, 2);
  u16 size = tmp[0] << 8 | tmp[1];

  // size seems to be including the size field!
  *len = size - 2;

  return WIZNET_RESULT_OK;
}

void wiznet_eth_recv_data(u16 offset, u08 *buf, u16 len)
{
  wiz_io_rx_buffer_read(0, offset + 2, buf, len);
}

u08  wiznet_eth_recv_end(u16 size)
{
  wiz_io_rx_buffer_confirm(0, size + 2);

  // trigger next receive
  return exec_cmd(0, WIZ_SOCKET_CMD_RECV);
}
