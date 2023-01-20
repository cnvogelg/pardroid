#ifndef PAMELA_SOCK_H
#define PAMELA_SOCK_H

/* perform I/O on sockets. perform a single step.
   return TRUE if done or FALSE if more work is needed. */
BOOL pamela_sock_work(pamela_engine_t *eng);

/* engine had a idle timeout */
void pamela_sock_timeout(pamela_engine_t *eng);

/* pamela reported an event. update sockets */
void pamela_sock_event(pamela_engine_t *eng);

/* shutdown client associated with socket */
void pamela_sock_shutdown_client(pamela_engine_t *eng, pamela_client_t *pc);

void pamela_sock_shutdown_socket(pamela_engine_t *eng, pamela_socket_t *sock);

void pamela_sock_cancel_read_write(pamela_engine_t *eng, pamela_socket_t *sock);

#endif
