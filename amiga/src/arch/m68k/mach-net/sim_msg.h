#ifndef SIM_MSG_H
#define SIM_MSG_H

#define SM_TYPE_CMD     0
#define SM_TYPE_STATUS  1

#define SM_STATUS_CONNECTED     1
#define SM_STATUS_ACK_IRQ       2

struct sim_msg {
  struct Message msg;
  UBYTE          type;
  UBYTE          value;
  UWORD          tx_size;
  UWORD          rx_size;
  APTR           data_ptr;
};
typedef struct sim_msg sim_msg_t;

struct sim_msg_handle {
  struct MsgPort *port;
  ULONG           sig_mask;
};
typedef struct sim_msg_handle sim_msg_handle_t;

int sim_msg_server_init(sim_msg_handle_t *handle);
void sim_msg_server_exit(sim_msg_handle_t *handle);

struct sim_msg *sim_msg_get(sim_msg_handle_t *handle);
void sim_msg_reply(sim_msg_t *msg);

int sim_msg_client_init(sim_msg_handle_t *handle);
void sim_msg_client_exit(sim_msg_handle_t *handle);

int sim_msg_client_send(sim_msg_handle_t *handle, struct sim_msg *msg);

int sim_msg_client_do_cmd(sim_msg_handle_t *handle, UBYTE cmd, void *data,
                          UWORD tx_size, UWORD rx_size);
int sim_msg_client_do_status(sim_msg_handle_t *handle, UBYTE *status);

#endif
