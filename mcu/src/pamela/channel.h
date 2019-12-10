#ifndef CHANNEL_H
#define CHANNEL_H

#include "proto_shared.h"
#include "handler.h"

/* channel status is a bit field */
#define CHANNEL_STATUS_ATTACHED     0x01
#define CHANNEL_STATUS_OPENED       0x02
#define CHANNEL_STATUS_NO_ERROR     0x04
#define CHANNEL_STATUS_RX_PENDING   0x10
#define CHANNEL_STATUS_RX_REQUEST   0x20
#define CHANNEL_STATUS_RX_OP        0x40
#define CHANNEL_STATUS_TX_OP        0x80

// attached & opened & no error
#define CHANNEL_MASK_VALID          0x07

/* error codes for channel */
#define CHANNEL_ERROR_NONE          0x00
#define CHANNEL_ERROR_NOT_ATTACHED  0x01
#define CHANNEL_ERROR_ALREADY_OPEN  0x02
#define CHANNEL_ERROR_NOT_OPEN      0x03
#define CHANNEL_ERROR_INVALID_MTU   0x04
#define CHANNEL_ERROR_RX_EMPTY      0x05
#define CHANNEL_ERROR_TX_TOO_LARGE  0x06 
#define CHANNEL_ERROR_DUPL_RX_REQ   0x07
#define CHANNEL_ERROR_DUPL_HANDLER  0x08
#define CHANNEL_ERROR_HAS_ERROR     0x09


struct channel {
  u08            status;       /* channel status */
  u16            error_code;   /* detailed error code */
  u16            mtu;          /* current MTU */
  handler_ptr_t  handler;      /* assigned handler or NULL */

  u16            max_rx_size;
  u16            rx_size;
  u16            tx_size;

  u32            rx_offset;
  u32            tx_offset;

  u16            rx_frag_off;
  u16            tx_frag_off;
  u08           *rx_buf;
};
typedef struct channel channel_t;
typedef channel_t *channel_ptr_t;

extern channel_t channel_map[PROTO_MAX_CHANNEL];

/* called by pamela */
extern void channel_all_init(void);
extern void channel_all_work(void);

/* called by user code to manage handler */
extern u08  channel_attach(u08 id, handler_ptr_t handler);
extern u08  channel_detach(u08 id);

/* called by proto api handler */
extern void channel_open(u08 id);
extern void channel_close(u08 id);
extern void channel_reset(u08 id);

extern void channel_set_mtu(u08 id, u16 mtu);
extern u16  channel_get_properties(u08 id);
extern u16  channel_get_def_mtu(u08 id);

/* called by handler code */
extern void channel_set_error_code(u08 id, u16 error_code);
extern void channel_send(u08 id, u16 size, u08 *buf);

#endif
