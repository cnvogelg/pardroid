#ifndef SIM_PKT_H
#define SIM_PKT_H

#include "udp.h"

/* status codes */
#define SIM_PKT_OK                0
#define SIM_PKT_SIGNAL            1
#define SIM_PKT_TIME_OUT          2
#define SIM_PKT_ERROR_MEMORY      -1
#define SIM_PKT_ERROR_UDP_INIT    -2
#define SIM_PKT_ERROR_UDP_ADDR    -3
#define SIM_PKT_ERROR_UDP_SOCKET  -4
#define SIM_PKT_ERROR_TIMER_INIT  -5
#define SIM_PKT_ERROR_TIMER_SIG   -6
#define SIM_PKT_ERROR_UDP_WAIT    -7
#define SIM_PKT_ERROR_UDP_RECV    -8
#define SIM_PKT_ERROR_UDP_SEND    -9
#define SIM_PKT_ERROR_WRONG_PEER  -10
#define SIM_PKT_ERROR_TOO_LARGE   -11
#define SIM_PKT_ERROR_NO_MAGIC    -12
#define SIM_PKT_ERROR_WRONG_TYPE  -13
#define SIM_PKT_ERROR_NOT_CONNECTED -14

/* type */
#define SIM_PKT_TYPE_STATUS       0
#define SIM_PKT_TYPE_CMD          1

/* status bits */
#define SIM_PKT_STATUS_CONNECTED  1
#define SIM_PKT_STATUS_ACK_IRQ    2

struct sim_pkt_handle {
  struct udp_handle    udp;
  struct sockaddr_in   my_addr;
  struct sockaddr_in   peer_addr;
  struct sockaddr_in   cur_addr;
  int                  sock_fd;
  int                  time_out_cnt;
  UWORD                port;
  ULONG                buf_size;
  UBYTE               *buf_ptr;
  UBYTE                status;
};
typedef struct sim_pkt_handle sim_pkt_handle_t;

struct sim_pkt {
  UBYTE          type;
  UBYTE          value;
  UBYTE         *buf_ptr;
  ULONG          buf_size;
};
typedef struct sim_pkt sim_pkt_t;

int  sim_pkt_init(sim_pkt_handle_t *hnd, ULONG buf_size, UWORD port);
void sim_pkt_exit(sim_pkt_handle_t *hnd);

int sim_pkt_recv(sim_pkt_handle_t *hnd, sim_pkt_t *pkt, ULONG *extra_sig_mask);
int sim_pkt_send(sim_pkt_handle_t *hnd, sim_pkt_t *pkt);

#endif
