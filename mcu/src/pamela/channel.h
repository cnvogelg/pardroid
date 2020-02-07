#ifndef CHANNEL_H
#define CHANNEL_H

#include "proto_shared.h"
#include "handler.h"

/* channel status is a bit field */
#define CHANNEL_STATE_ATTACHED     0x01
#define CHANNEL_STATE_OPENED       0x02
#define CHANNEL_STATE_ERROR        0x04
#define CHANNEL_STATE_RX_OP        0x40
#define CHANNEL_STATE_TX_OP        0x80
#define CHANNEL_STATE_OP_MASK      0xc0

// attached & opened & no error
#define CHANNEL_MASK_VALID          0x07

/* error codes for channel */
#define CHANNEL_ERROR_NONE          0x00
#define CHANNEL_ERROR_ATTACH        0x01
#define CHANNEL_ERROR_OPEN          0x02
#define CHANNEL_ERROR_HANDLER_OPEN  0x03
#define CHANNEL_ERROR_OP_RUNNING    0x04
#define CHANNEL_ERROR_OP_STATE      0x05
#define CHANNEL_ERROR_NO_CHANNEL    0x06

struct channel {
  u08            state;        /* channel state */
  u16            error_code;   /* detailed error code */
  u16            mtu;          /* current MTU */
  handler_ptr_t  handler;      /* assigned handler or NULL */

  u16            tr_num_words; // current transfer size
  u16            tr_got_words; // already received words
  u32            tr_offset;    // current transfer offset
};
typedef struct channel channel_t;
typedef channel_t *channel_ptr_t;

/* called by user */
extern void channel_init(channel_ptr_t chn, handler_ptr_t handler);

/* called by pamela */
extern void channel_work(u08 id);
extern void channel_work_all(void);

/* called by proto api handler */
extern void channel_open(u08 id);
extern void channel_close(u08 id);
extern void channel_reset(u08 id);

extern void channel_set_mtu(u08 id, u16 mtu);
extern u16  channel_get_mode(u08 id);
extern u16  channel_get_def_mtu(u08 id);
extern u16  channel_get_mtu(u08 id);
extern void channel_set_offset(u08 id, u32 offset);
extern u32  channel_get_offset(u08 id);

/* read and clear error code. reset error flag */
extern u16  channel_get_error_code(u08 id);
/* set error code. raise error flag */
extern void channel_set_error_code(u08 id, u16 error_code);

/* read op */
extern u16 channel_read_begin(u08 id);
extern u08 *channel_read_chunk_begin(u08 id, u16 *ret_size);
extern void channel_read_chunk_end(u08 id);

/* write op */
extern void channel_write_begin(u08 id, u16 size);
extern u08 *channel_write_chunk_begin(u08 id, u16 *ret_size);
extern void channel_write_chunk_end(u08 id);

extern void channel_transfer_cancel(u08 id);

/* channel api - needs to be implemented by firmware */
extern channel_ptr_t channel_api_get_channel(u08 id);
// bitmask with available channels
extern u16 channel_api_get_mask(void); 

#endif
