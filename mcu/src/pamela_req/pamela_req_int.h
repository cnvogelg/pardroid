#ifndef PAMELA_REQ_INT_H
#define PAMELA_REQ_INT_H

/* states of a req slot */
#define PAMELA_REQ_STATE_IDLE              0
#define PAMELA_REQ_STATE_IN_WRITE          1
#define PAMELA_REQ_STATE_BUSY              2
#define PAMELA_REQ_STATE_READ_WAIT         3
#define PAMELA_REQ_STATE_IN_READ           4

/* slot instance data */
struct pamela_req_slot {
  u08  state;
  u08 *reply_buf;
  u16  reply_size;
};
typedef struct pamela_req_slot pamela_req_slot_t;

/* generic service handler for all req services */

extern u08 pamela_req_open(u08 chan, u16 port);
extern u08 pamela_req_close(u08 chan);
extern u08 pamela_req_reset(u08 chan);
extern u08 pamela_req_read_request(u08 chan, u08 **buf, u16 *size);
extern void pamela_req_read_done(u08 chan, u08 *buf, u16 size);
extern u08 pamela_req_write_request(u08 chan, u08 **buf, u16 *size);
extern void pamela_req_write_done(u08 chan, u08 *buf, u16 size);

#endif
