#ifndef REQUEST_H
#define REQUEST_H

#include <exec/ports.h>

#define PB_REQ_MESSAGE_READ         0
#define PB_REQ_MESSAGE_WRITE        1

#define PB_REQ_RET_OK               0
#define PB_REQ_RET_INVALID_CMD      1
#define PB_REQ_RET_INVALID_CHANNEL  3
#define PB_REQ_RET_CHANNEL_NOT_OPEN 4
#define PB_REQ_RET_PROTO_ERROR      5

struct request {
  struct Message   msg;
  UWORD            cmd;
  UBYTE            channel;
  UBYTE            pad0;
  UBYTE            error;
  UBYTE            sub_error;
  APTR             data;
  UWORD            length;
  UWORD            actual;
};
typedef struct request request_t;

extern request_t *request_create(struct MsgPort *reply_port);
extern void request_delete(request_t *req);

#endif
