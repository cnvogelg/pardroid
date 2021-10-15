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

#include "sim_msg.h"

#define RET_OK                  0
#define RET_RAK_INVALID         1
#define RET_TIMEOUT             2
#define RET_SLAVE_ERROR         3
#define RET_MSG_TOO_LARGE       4
#define RET_DEVICE_BUSY         5

void proto_low_config_port(struct pario_port *port)
{
    // nothing to be done
}

static int do_cmd(struct partio_port *port, volatile UBYTE *timeout_flag, UBYTE cmd,
                  void *buf, UWORD tx_size, UWORD_rx_size)
{
  sim_msg_handle_t *hnd = &port->handle->hnd_cmd;
  int res = sim_msg_client_do_cmd(hnd, cmd, buf, tx_size, rx_size);
  if(res != 0) {
    return RET_SLAVE_ERROR;
  }
  if(*timeout) {
    return RET_TIMEOUT;
  }
  return RET_OK;
}

ASM int proto_low_action(REG(a0, struct pario_port *port),
                         REG(a1, volatile UBYTE *timeout_flag),
                         REG(d0, UBYTE cmd))
{
  return do_cmd(port, timeout_flag, cmd, NULL, 0, 0);
}

ASM int proto_low_action_no_busy(REG(a0, struct pario_port *port),
                                 REG(a1, volatile UBYTE *timeout_flag),
                                 REG(d0, UBYTE cmd))
{
  return do_cmd(port, timeout_flag, cmd, NULL, 0, 0);
}

ASM int proto_low_action_bench(REG(a0, struct pario_port *port),
                               REG(a1, volatile UBYTE *timeout_flag),
                               REG(a2, struct cb_data *cbd),
                               REG(d0, UBYTE cmd))
{
  return do_cmd(port, timeout_flag, cmd, NULL, 0, 0);
}

ASM int proto_low_read_word(REG(a0, struct pario_port *port),
                            REG(a1, volatile UBYTE *timeout_flag),
                            REG(d0, UBYTE cmd),
                            REG(a2, UWORD *data))
{
  return do_cmd(port, timeout_flag, cmd, data, 0, 2);
}

ASM int proto_low_write_word(REG(a0, struct pario_port *port),
                             REG(a1, volatile UBYTE *timeout_flag),
                             REG(d0, UBYTE cmd),
                             REG(a2, UWORD *data))
{
  return do_cmd(port, timeout_flag, cmd, data, 2, 0);
}

ASM int proto_low_read_long(REG(a0, struct pario_port *port),
                            REG(a1, volatile UBYTE *timeout_flag),
                            REG(d0, UBYTE cmd),
                            REG(a2, ULONG *data))
{
  return do_cmd(port, timeout_flag, cmd, data, 0, 4);
}

ASM int proto_low_write_long(REG(a0, struct pario_port *port),
                             REG(a1, volatile UBYTE *timeout_flag),
                             REG(d0, UBYTE cmd),
                             REG(a2, ULONG *data))
{
  return do_cmd(port, timeout_flag, cmd, data, 4, 0);
}

ASM int proto_low_read_block(REG(a0, struct pario_port *port),
                             REG(a1, volatile UBYTE *timeout_flag),
                             REG(d0, UBYTE cmd),
                             REG(a2, proto_iov_t *msgiov),
                             REG(d1, UWORD num_words))
{
  ULONG size = num_words * 2;

  // recv but do not copy buffer
  D(("read block\n"));
  int result = msg_transmit(port, cmd, NULL, 0, size);
  if(result != RET_OK) {
    return result;
  }
  D(("read block: done\n"));

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

  D("ret OK\n");
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
