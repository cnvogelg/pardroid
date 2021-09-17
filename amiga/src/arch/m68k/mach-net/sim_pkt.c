#include <proto/exec.h>
#include <proto/dos.h>
#include <libraries/bsdsocket.h>

#include "sim_pkt.h"

#define MAGIC 0x41504200 // 'APB\0'
#define MAX_TIME_OUT 5

/* packet type codes */
#define SIM_PKT_PROTO_CONNECT      1
#define SIM_PKT_PROTO_CON_ACK      2
#define SIM_PKT_PROTO_DISCONNECT   3
#define SIM_PKT_PROTO_DIS_ACK      4
#define SIM_PKT_PROTO_CMD_REQ      5
#define SIM_PKT_PROTO_CMD_REP      6
#define SIM_PKT_PROTO_STATUS       7
#define SIM_PKT_PROTO_SRV_ALIVE    8
#define SIM_PKT_PROTO_LAST         9

struct proto_packet {
  ULONG magic;
  UBYTE type;
  UBYTE value;
  UWORD size;
};

int  sim_pkt_init(sim_pkt_handle_t *hnd, ULONG buf_size, UWORD port)
{
  hnd->port = port;
  hnd->buf_size = buf_size + sizeof(struct proto_packet);
  hnd->sock_fd = -1;
  hnd->buf_ptr = NULL;
  hnd->udp.socketBase = NULL;
  hnd->status = 0;

  /* alloc buf */
  hnd->buf_ptr = AllocVec(hnd->buf_size, MEMF_PUBLIC | MEMF_CLEAR);
  if(hnd->buf_ptr == NULL) {
    return SIM_PKT_ERROR_MEMORY;
  }

  /* setup udp */
  if(udp_init(&hnd->udp,(struct ExecBase *)SysBase)!=0) {
    sim_pkt_exit(hnd);
    return SIM_PKT_ERROR_UDP_INIT;
  }
  /* setup my addr */
  if(udp_addr_setup(&hnd->udp, &hnd->my_addr, "0.0.0.0", port)!=0) {
    sim_pkt_exit(hnd);
    return SIM_PKT_ERROR_UDP_ADDR;
  }

  /* setup socket */
  hnd->sock_fd = udp_open(&hnd->udp, &hnd->my_addr);
  if(hnd->sock_fd < 0) {
    sim_pkt_exit(hnd);
    return SIM_PKT_ERROR_UDP_SOCKET;
  }

  return SIM_PKT_OK;
}

void sim_pkt_exit(sim_pkt_handle_t *hnd)
{
  /* udp socket */
  if(hnd->sock_fd >= 0) {
    udp_close(&hnd->udp, hnd->sock_fd);
    hnd->sock_fd = -1;
  }

  /* udp exit */
  if(hnd->udp.socketBase != NULL) {
    udp_exit(&hnd->udp);
    hnd->udp.socketBase = NULL;
  }

  /* free buf */
  if(hnd->buf_ptr != NULL) {
    FreeVec(hnd->buf_ptr);
    hnd->buf_ptr = NULL;
  }
}

static int send_pkt(sim_pkt_handle_t *hnd, struct proto_packet *pkt, ULONG data_size)
{
  if((hnd->status & SIM_PKT_STATUS_CONNECTED)==0) {
    return SIM_PKT_ERROR_NOT_CONNECTED;
  }

  pkt->magic = MAGIC;
  pkt->size = data_size;

  ULONG size = sizeof(struct proto_packet) + data_size;
  if(udp_send(&hnd->udp, hnd->sock_fd, &hnd->peer_addr, hnd->buf_ptr, size)<0) {
    return SIM_PKT_ERROR_UDP_SEND;
  }
}

static int send_alive(sim_pkt_handle_t *hnd)
{
  /* send server alive */
  struct proto_packet *reply = (struct proto_packet *)hnd->buf_ptr;
  reply->type = SIM_PKT_PROTO_SRV_ALIVE;
  reply->value = 0;
  return send_pkt(hnd, reply, 0);
}

static void make_status_pkt(sim_pkt_handle_t *hnd, sim_pkt_t *ret_pkt)
{
  ret_pkt->type = SIM_PKT_TYPE_STATUS;
  ret_pkt->value = hnd->status;
  ret_pkt->buf_size = 0;
}

static int handle_connect(sim_pkt_handle_t *hnd, struct proto_packet *pkt,
                          sim_pkt_t *ret_pkt)
{
  hnd->status = SIM_PKT_STATUS_CONNECTED;
  hnd->time_out_cnt = 0;
  hnd->peer_addr = hnd->cur_addr;

  /* answer with CON_ACK */
  struct proto_packet *reply = (struct proto_packet *)hnd->buf_ptr;
  reply->type = SIM_PKT_PROTO_CON_ACK;
  reply->value = 0;
  int res = send_pkt(hnd, reply, 0);
  if(res < 0) {
    return res;
  }

  make_status_pkt(hnd, ret_pkt);
  return SIM_PKT_OK;
}

static int handle_disconnect(sim_pkt_handle_t *hnd, struct proto_packet *pkt,
                             sim_pkt_t *ret_pkt)
{
  /* answer with DIS_ACK */
  struct proto_packet *reply = (struct proto_packet *)hnd->buf_ptr;
  reply->type = SIM_PKT_PROTO_DIS_ACK;
  reply->value = 0;
  int res = send_pkt(hnd, reply, 0);
  if(res < 0) {
    return res;
  }

  hnd->status = 0;
  hnd->time_out_cnt = 0;

  make_status_pkt(hnd, ret_pkt);
  return SIM_PKT_OK;
}

static int handle_cmd_rep(sim_pkt_handle_t *hnd, struct proto_packet *pkt,
                          sim_pkt_t *ret_pkt, int pkt_size)
{
  hnd->time_out_cnt = 0;

  /* does fit in return packet? */
  ULONG data_size = pkt_size - sizeof(struct proto_packet);
  if(ret_pkt->buf_size < data_size) {
    return SIM_PKT_ERROR_TOO_LARGE;
  }

  ret_pkt->type = SIM_PKT_TYPE_CMD;
  ret_pkt->value = pkt->value;
  ret_pkt->buf_size = data_size;
  if(data_size > 0) {
    memcpy(ret_pkt->buf_ptr, hnd->buf_ptr + sizeof(struct proto_packet), data_size);
  } else {
    ret_pkt->buf_ptr = NULL;
  }

  return SIM_PKT_OK;
}

static int handle_status(sim_pkt_handle_t *hnd, struct proto_packet *pkt,
                         sim_pkt_t *ret_pkt)
{
  hnd->time_out_cnt = 0;

  if(pkt->value == 0) {
    hnd->status &= ~SIM_PKT_STATUS_ACK_IRQ;
  } else {
    hnd->status |= SIM_PKT_STATUS_ACK_IRQ;
  }

  send_alive(hnd);

  make_status_pkt(hnd, ret_pkt);
  return SIM_PKT_OK;
}

static int handle_time_out(sim_pkt_handle_t *hnd, sim_pkt_t *ret_pkt)
{
  /* check idle timeout from client */
  if(hnd->status & SIM_PKT_STATUS_CONNECTED) {
    hnd->time_out_cnt++;
    if(hnd->time_out_cnt>=MAX_TIME_OUT) {
      /* auto disconnect */
      hnd->status = 0;
      hnd->time_out_cnt = 0;

      make_status_pkt(hnd, ret_pkt);
      return SIM_PKT_OK;
    } else {
      int res = send_alive(hnd);
      if(res < 0) {
        return res;
      }
    }
  }

  return SIM_PKT_TIME_OUT;
}

static int handle_packet(sim_pkt_handle_t *hnd, sim_pkt_t *ret_pkt, int pkt_size)
{
  UBYTE *buf = hnd->buf_ptr;

  /* first check magic */
  struct proto_packet *pkt = (struct proto_packet *)buf;
  if(pkt->magic != MAGIC) {
    return SIM_PKT_ERROR_NO_MAGIC;
  }
  /* packet dispatch */
  switch(pkt->type) {
    case SIM_PKT_PROTO_CONNECT:
      return handle_connect(hnd, pkt, ret_pkt);
    case SIM_PKT_PROTO_DISCONNECT:
      return handle_disconnect(hnd, pkt, ret_pkt);
    case SIM_PKT_PROTO_CMD_REP:
      return handle_cmd_rep(hnd, pkt, ret_pkt, pkt_size);
    case SIM_PKT_PROTO_STATUS:
      return handle_status(hnd, pkt, ret_pkt);
    default:
      return SIM_PKT_ERROR_WRONG_TYPE;
  }
}

int sim_pkt_recv(sim_pkt_handle_t *hnd, sim_pkt_t *ret_pkt, ULONG *extra_sig_mask)
{
  /* receive packet */
  int res = udp_wait_recv(&hnd->udp, hnd->sock_fd, 1, 0, extra_sig_mask);

  /* error */
  if(res < 0) {
    return SIM_PKT_ERROR_UDP_WAIT;
  }

  /* check sig mask returned */
  if((extra_sig_mask != NULL) && (*extra_sig_mask != 0)) {
    return SIM_PKT_SIGNAL;
  }

  /* udp recv avaialable */
  else if(res > 0) {
    /* recv packet */
    res = udp_recv(&hnd->udp, hnd->sock_fd, &hnd->cur_addr, hnd->buf_ptr, hnd->buf_size);
    if(res < 0) {
      return SIM_PKT_ERROR_UDP_RECV;
    }
    /* check_size? */
    if(res > hnd->buf_size) {
      return SIM_PKT_ERROR_TOO_LARGE;
    }
    return handle_packet(hnd, ret_pkt, res);
  }
  /* time out */
  else {
    return handle_time_out(hnd, ret_pkt);
  }
}

int sim_pkt_send(sim_pkt_handle_t *hnd, sim_pkt_t *pkt)
{
  /* its always a cmd packet */
  struct proto_packet *raw_pkt = (struct proto_packet *)hnd->buf_ptr;
  raw_pkt->type = SIM_PKT_PROTO_CMD_REQ;
  raw_pkt->value = pkt->value;

  /* append data to packet */
  ULONG data_size = pkt->buf_size;
  raw_pkt->size = (UWORD)data_size;
  if(data_size > 0) {
    UBYTE *tgt_buf = hnd->buf_ptr + sizeof(struct proto_packet);
    memcpy(tgt_buf, pkt->buf_ptr, data_size);
  }

  return send_pkt(hnd, raw_pkt, data_size);
}
