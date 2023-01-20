#ifndef PAMELA_ENGINE_INT_H
#define PAMELA_ENGINE_INT_H

struct pamela_client {
  struct MinNode node;
  pamela_req_t  *request; /* each client has an own request */
  UWORD          channel_mask; /* which channels are open */
};
typedef struct pamela_client pamela_client_t;

struct pamela_socket {
  pamela_channel_t *channel;
  pamela_client_t  *client;
  pamela_req_t     *read_req;
  pamela_req_t     *write_req;
  pamela_req_t     *cmd_req;
  UWORD             last_status;
};
typedef struct pamela_socket pamela_socket_t;

/* private engine struct */
struct pamela_engine {
  pamela_handle_t  *pamela;
  struct Library   *sys_base;
  struct MsgPort   *req_port;
  ULONG             port_sigmask;
  /* all the clients registered by ioreq */
  struct MinList    clients;
  /* all channels have an own socket */
  pamela_socket_t  *sockets;
  UWORD             num_sockets;
  /* quit signal */
  struct Task      *task;
  BYTE              quit_signal;
};

#endif
