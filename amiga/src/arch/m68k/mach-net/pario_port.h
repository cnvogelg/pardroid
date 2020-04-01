#ifndef PARIO_PORT_H
#define PARIO_PORT_H

struct pario_handle;

struct pario_port {
  struct pario_handle  *handle; 
};

struct pario_handle {
  struct Library *      sysBase;
  struct Library *      socketBase;
  int                   my_sock_fd;
  int                   peer_sock_fd;
  struct sockaddr_in    my_addr;
  struct sockaddr_in    peer_addr;

  struct Task *         sig_task;
  BYTE                  signal;

  struct pario_port     port;
  BYTE *                msg_buf;
  ULONG                 msg_max;
  ULONG                 msg_seq;
};

#endif
