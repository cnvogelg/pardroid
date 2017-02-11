#ifndef PROTO_H
#define PROTO_H

#include "pario.h"
#include "timer.h"

struct proto_handle;

#define PROTO_CMD_IDLE        0
#define PROTO_CMD_PING        0x10
#define PROTO_CMD_RESET       0x1f

#define PROTO_RET_OK            0
#define PROTO_RET_RAK_INVALID   1
#define PROTO_RET_TIMEOUT       2
#define PROTO_RET_SLAVE_ERROR   3

extern struct proto_handle *proto_init(struct pario_port *port, struct timer_handle *th);
extern void proto_exit(struct proto_handle *ph);

extern int proto_ping(struct proto_handle *ph);
extern int proto_reset(struct proto_handle *ph);
extern int proto_test_read(struct proto_handle *ph, UWORD *data);
extern int proto_test_write(struct proto_handle *ph, UWORD *data);

extern const char *proto_perror(int res);

#endif
