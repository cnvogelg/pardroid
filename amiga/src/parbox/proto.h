#ifndef PROTO_H
#define PROTO_H

#include "pario.h"
#include "timer.h"

struct proto_handle;

#define NUM_REG             16
#define REG_TEST            0

#define PROTO_CMD_IDLE            0x00
#define PROTO_CMD_PING            0x10
#define PROTO_CMD_RESET           0x1f
#define PROTO_CMD_REG_WRITE_BASE  0x50
#define PROTO_CMD_REG_WRITE_LAST  0x6f
#define PROTO_CMD_REG_READ_BASE   0xd0
#define PROTO_CMD_REG_READ_LAST   0xef

#define PROTO_RET_OK            0
#define PROTO_RET_RAK_INVALID   1
#define PROTO_RET_TIMEOUT       2
#define PROTO_RET_SLAVE_ERROR   3
#define PROTO_RET_INVALID_REG   99

extern struct proto_handle *proto_init(struct pario_port *port, struct timer_handle *th);
extern void proto_exit(struct proto_handle *ph);

extern int proto_ping(struct proto_handle *ph);
extern int proto_reset(struct proto_handle *ph);

extern int proto_reg_read(struct proto_handle *ph, UBYTE reg, UWORD *data);
extern int proto_reg_write(struct proto_handle *ph, UBYTE reg, UWORD *data);

extern const char *proto_perror(int res);

#endif
