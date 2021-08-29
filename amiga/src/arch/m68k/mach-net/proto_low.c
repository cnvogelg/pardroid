#define __NOLIBBASE__
#include <proto/exec.h>
#include <libraries/bsdsocket.h>

#include "autoconf.h"

#ifdef CONFIG_DEBUG_PARIO
#define KDEBUG
#endif

#include "compiler.h"
#include "debug.h"

#include "pario.h"
#include "proto_iov.h"
#include "proto_low.h"
#include "pario_port.h"
#include "proto_shared.h"
#include "udp.h"

#define RET_OK                  0
#define RET_RAK_INVALID         1
#define RET_TIMEOUT             2
#define RET_SLAVE_ERROR         3
#define RET_MSG_TOO_LARGE       4
#define RET_DEVICE_BUSY         5

void proto_low_config_port(struct pario_port *port)
{
    //
}

static int msg_transmit(struct pario_port *port, UBYTE cmd, void *data,
                        ULONG tx_size, ULONG rx_size)
{
  struct pario_handle *ph = port->handle;
  int res;

  // prepare packet
  ULONG magic = 0x41504200 | cmd; // 'APB' + cmd
  ULONG *ptr = (ULONG *)ph->msg_buf;
  *ptr = magic;
  *(ptr+1) = ph->msg_seq;

  if(tx_size > (ph->msg_max - 8)) {
    D(("transmit: tx_size too large: %ld\n", tx_size));
    return RET_MSG_TOO_LARGE;
  }
  if((tx_size > 0) && (data != NULL)) {
    memcpy(ph->msg_buf + 8, data, tx_size);
  }
  ULONG msg_size = tx_size + 8;

  // send packet
  struct udp_handle *udp = &ph->udp_handle;
  res = udp_send(udp, ph->peer_sock_fd, &ph->peer_addr, ph->msg_buf, msg_size);
  if(res != 0) {
    D(("transmit: error udp_send!\n"));
    return RET_RAK_INVALID;
  }

  // wait for response
  res = udp_wait_recv(udp, ph->peer_sock_fd, 0, 500000UL, NULL);
  if(res == 0) {
    D(("transmit: timeout!\n"));
    return RET_TIMEOUT;
  }
  if(res < 0) {
    D(("transmit: wait error!\n"));
    return RET_RAK_INVALID;
  }

  // read response packet
  res = udp_recv(udp, ph->peer_sock_fd, NULL, ph->msg_buf, ph->msg_max);
  if(res < 0) {
    D(("transmit: recv error!\n"));
    return RET_RAK_INVALID;
  }

  // too small?
  if(res < 8) {
    D(("transmit: recv no header!\n"));
    return RET_RAK_INVALID;
  }

  // check magic
  if(*ptr != magic) {
    D(("transmit: wrong magic!\n"));
    return RET_RAK_INVALID;
  }
  if(*(ptr+1) != ph->msg_seq) {
    D(("transmit: wrong seq!\n"));
    return RET_RAK_INVALID;
  }

  // data returned?
  ULONG data_size = res - 8;
  if(data_size != rx_size) {
    D(("transmit: rx_size wrong: %ld != %ld\n", data_size, rx_size));
    return RET_RAK_INVALID;
  }

  if((data_size > 0) && (data != NULL)) {
    memcpy(data, ph->msg_buf + 8, data_size);
  }

  ph->msg_seq++;

  return RET_OK;
}

ASM int proto_low_action(REG(a0, struct pario_port *port),
                         REG(a1, volatile UBYTE *timeout_flag),
                         REG(d0, UBYTE cmd))
{
  return msg_transmit(port, cmd, NULL, 0, 0);
}

ASM int proto_low_action_no_busy(REG(a0, struct pario_port *port),
                                 REG(a1, volatile UBYTE *timeout_flag),
                                 REG(d0, UBYTE cmd))
{
  return msg_transmit(port, cmd, NULL, 0, 0);
}

ASM int proto_low_action_bench(REG(a0, struct pario_port *port),
                               REG(a1, volatile UBYTE *timeout_flag),
                               REG(a2, struct cb_data *cbd),
                               REG(d0, UBYTE cmd))
{
  int result = msg_transmit(port, cmd, NULL, 0, 0);
  return result;
}

ASM int proto_low_read_word(REG(a0, struct pario_port *port),
                            REG(a1, volatile UBYTE *timeout_flag),
                            REG(d0, UBYTE cmd),
                            REG(a2, UWORD *data))
{
  return msg_transmit(port, cmd, data, 0, 2);
}

ASM int proto_low_write_word(REG(a0, struct pario_port *port),
                             REG(a1, volatile UBYTE *timeout_flag),
                             REG(d0, UBYTE cmd),
                             REG(a2, UWORD *data))
{
 return msg_transmit(port, cmd, data, 2, 0);
}

ASM int proto_low_read_long(REG(a0, struct pario_port *port),
                            REG(a1, volatile UBYTE *timeout_flag),
                            REG(d0, UBYTE cmd),
                            REG(a2, ULONG *data))
{
  return msg_transmit(port, cmd, data, 0, 4);
}

ASM int proto_low_write_long(REG(a0, struct pario_port *port),
                             REG(a1, volatile UBYTE *timeout_flag),
                             REG(d0, UBYTE cmd),
                             REG(a2, ULONG *data))
{
  return msg_transmit(port, cmd, data, 4, 0);
}

ASM int proto_low_read_block(REG(a0, struct pario_port *port),
                             REG(a1, volatile UBYTE *timeout_flag),
                             REG(d0, UBYTE cmd),
                             REG(a2, proto_iov_t *msgiov),
                             REG(d1, UWORD num_words))
{
  ULONG size = num_words * 2;
  struct pario_handle *ph = port->handle;

  // check size
  if((size + 8) > ph->msg_max) {
      struct pario_handle *ph = port->handle;
      D(("read_block: size too large: %ld\n", size));
      return RET_MSG_TOO_LARGE;
  }

  // recv but do not copy buffer
  int result = msg_transmit(port, cmd, NULL, 0, size);
  if(result != RET_OK) {
    return result;
  }

  // copy to iov
  UBYTE *buf = (UBYTE *)(ph->msg_buf + 8);
  UBYTE *ptr;
  while(msgiov != NULL) {
    ptr = msgiov->data;
    size = msgiov->num_words * 2;
    memcpy(ptr, buf, size);
    buf += size;
    msgiov = msgiov->next;
  }

  return RET_OK;
}

ASM int proto_low_write_block(REG(a0, struct pario_port *port),
                              REG(a1, volatile UBYTE *timeout_flag),
                              REG(d0, UBYTE cmd),
                              REG(a2, proto_iov_t *msgiov),
                              REG(d1, UWORD num_words))
{
  ULONG size = num_words * 2;
  struct pario_handle *ph = port->handle;

  // check size
  if((size + 8) > ph->msg_max) {
      struct pario_handle *ph = port->handle;
      D(("write_block: size too large: %ld\n", size));
      return RET_MSG_TOO_LARGE;
  }

  // copy to iov
  UBYTE *buf = (UBYTE *)(ph->msg_buf + 8);
  UBYTE *ptr;
  while(msgiov != NULL) {
    ptr = msgiov->data;
    size = msgiov->num_words * 2;
    memcpy(buf, ptr, size);
    buf += size;
    msgiov = msgiov->next;
  }

  // recv but do not copy buffer
  return msg_transmit(port, cmd, NULL, size, 0);
}
