#ifndef REQUEST_H
#define REQUEST_H

#include <exec/ports.h>

#define PB_REQ_MESSAGE_READ         0
#define PB_REQ_MESSAGE_READ_VEC     1
#define PB_REQ_MESSAGE_WRITE        2
#define PB_REQ_MESSAGE_WRITE_VEC    3

struct request {
  struct Message   msg;
  UBYTE            cmd;
  UBYTE            channel;
  UBYTE            flags;
  UBYTE            error;
  APTR             data;
  UWORD            length;
  UWORD            actual;
};
typedef struct request request_t;

extern request_t *request_create(struct MsgPort *reply_port);
extern void request_delete(request_t *req);

#endif
