#include <proto/exec.h>
#include <clib/alib_protos.h>
#include "sim_msg.h"

#define PORT_NAME "pario_sim"

int sim_msg_server_init(struct sim_msg_handle *handle)
{
  handle->port = CreatePort(PORT_NAME, 0);
  if(handle->port == NULL) {
    return -1;
  }

  handle->sig_mask = 1 << handle->port->mp_SigBit;
  return 0;
}

void sim_msg_server_exit(struct sim_msg_handle *handle)
{
  if(handle->port != NULL) {
    DeletePort(handle->port);
    handle->port = NULL;
  }
}

struct sim_msg *sim_msg_get(struct sim_msg_handle *handle)
{
  struct sim_msg *msg = (struct sim_msg *)GetMsg(handle->port);
  return msg;
}

void sim_msg_reply(sim_msg_t *msg)
{
  ReplyMsg(&msg->msg);
}

int sim_msg_client_init(struct sim_msg_handle *handle)
{
  handle->port = CreateMsgPort();
  if(handle->port == NULL) {
    return -1;
  }

  handle->sig_mask = 1 << handle->port->mp_SigBit;
  return 0;
}

void sim_msg_client_exit(struct sim_msg_handle *handle)
{
  if(handle->port != NULL) {
    DeletePort(handle->port);
    handle->port = NULL;
  }
}

int sim_msg_client_send(struct sim_msg_handle *handle, struct sim_msg *msg)
{
  int result = 0;

  msg->msg.mn_ReplyPort = handle->port;

  Disable();
  struct MsgPort *port = FindPort(PORT_NAME);
  if(port != NULL) {
    PutMsg(port, &msg->msg);
  } else {
    result = -1;
  }
  Enable();

  return result;
}

int sim_msg_client_do_cmd(sim_msg_handle_t *handle, UBYTE cmd, void *data,
                          UWORD tx_size, UWORD rx_size)
{
  struct sim_msg msg;

  msg.type = SM_TYPE_CMD;
  msg.value = cmd;
  msg.tx_size = tx_size;
  msg.rx_size = rx_size;
  msg.data_ptr = data;

  // send message
  int result = sim_msg_client_send(handle, &msg);
  if(result < 0) {
    return result;
  }

  // wait for result
  Wait(handle->sig_mask);
  struct sim_msg *ret_msg = sim_msg_get(handle);
  if(ret_msg != &msg) {
    return -99;
  }

  return 0;
}

int sim_msg_client_do_status(struct sim_msg_handle *handle, UBYTE *status)
{
  struct sim_msg msg;

  msg.type = SM_TYPE_STATUS;
  msg.value = *status;

  // send message
  int result = sim_msg_client_send(handle, &msg);
  if(result < 0) {
    return result;
  }

  // wait for result
  Wait(handle->sig_mask);

  // got a msg
  struct sim_msg *ret_msg = sim_msg_get(handle);
  if(ret_msg != &msg) {
    return -99;
  }

  // update status
  *status = msg.value;
  return 0;
}