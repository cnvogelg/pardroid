#ifndef PAMELA_REQ_INT_H
#define PAMELA_REQ_INT_H

/* states of a req slot */
#define PAMELA_REQ_STATE_IDLE              0
#define PAMELA_REQ_STATE_IN_WRITE          1
#define PAMELA_REQ_STATE_POLLING           2
#define PAMELA_REQ_STATE_READ_WAIT         3
#define PAMELA_REQ_STATE_IN_READ           4

/* slot instance data */
struct pamela_req_slot {
  u08  state;
  pamela_buf_t buf;
  pamela_buf_t global_buf;
};
typedef struct pamela_req_slot pamela_req_slot_t;

/* generic service handler for all req services */

extern u08 pamela_req_open(u08 chan, u08 state, u16 port);
extern u08 pamela_req_close(u08 chan, u08 state);
extern u08 pamela_req_reset(u08 chan, u08 state);
extern u08 pamela_req_read_pre(u08 chan, u08 state, pamela_buf_t *buf);
extern u08 pamela_req_read_post(u08 chan, u08 state, pamela_buf_t *buf);
extern u08 pamela_req_write_pre(u08 chan, u08 state, pamela_buf_t *buf);
extern u08 pamela_req_write_post(u08 chan, u08 state, pamela_buf_t *buf);
extern void pamela_req_channel_task(u08 chan);

#endif
