#ifndef UDP_H
#define UDP_H

struct udp_handle {
    struct ExecBase *sysBase;
    struct Library *socketBase;
};

extern int udp_init(struct udp_handle *uh, struct ExecBase *sysBase);
extern void udp_exit(struct udp_handle *uh);

extern int udp_addr_setup(struct udp_handle *uh, struct sockaddr_in *addr,
                          char *name, UWORD port);
extern int udp_open(struct udp_handle *uh, struct sockaddr_in *bind_addr);
extern void udp_close(struct udp_handle *uh, int sock_fd);
extern int udp_send(struct udp_handle *uh, int sock_fd, struct sockaddr_in *peer_addr,
                    void *buffer, ULONG len);
extern int udp_recv(struct udp_handle *uh, int sock_fd, struct sockaddr_in *ret_peer_addr,
                    void *buffer, ULONG len);
extern int udp_wait_recv(struct udp_handle *uh, int sock_fd,
                         ULONG timeout_s, ULONG timeout_us,
                         ULONG *sigmask);

#endif
