#ifndef UDP_H
#define UDP_H

extern int udp_addr_setup(struct pario_handle *ph, struct sockaddr_in *addr,
                          char *name, UWORD port);
extern int udp_open(struct pario_handle *ph, struct sockaddr_in *bind_addr);
extern void udp_close(struct pario_handle *ph, int sock_fd);
extern int udp_send(struct pario_handle *ph, int sock_fd, struct sockaddr_in *peer_addr,
                    void *buffer, ULONG len);
extern int udp_recv(struct pario_handle *ph, int sock_fd, struct sockaddr_in *ret_peer_addr,
                    void *buffer, ULONG len);

#endif
