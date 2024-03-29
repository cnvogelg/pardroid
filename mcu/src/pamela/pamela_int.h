#ifndef PAMELA_INT_H
#define PAMELA_INT_H

#include "pamela.h"

#define PAMELA_CHANNEL_FLAG_TASK      1

struct pamela_service {
  /* the associated channel mask */
  u16 channels;
  /* flags */
  u08 flags;
  /* service instance id */
  u08 srv_id;
};
typedef struct pamela_service pamela_service_t;

struct pamela_channel {
  /* assigned handler (if any) */
  pamela_service_t *service;
  /* port */
  u16 port;
  /* current mtu */
  u16 mtu;

  /* current status. */
  u16 status;
  u16 error;

  /* pending rx request */
  u16 rx_org_size;
  pamela_buf_t rx_buf;
  /* pending tx request */
  u16 tx_org_size;
  pamela_buf_t tx_buf;

  /* id */
  u08 chan_id; /* global channel no */
  u08 slot_id; /* local slot id per service */
  u08 flags;
};
typedef struct pamela_channel pamela_channel_t;

/* unused slots are marked this way. 0..max_slots-1 is valid */
#define PAMELA_EMPTY_SLOT   0xff

extern pamela_channel_t *pamela_get_channel(u08 chn);

extern pamela_service_t *pamela_find_service(u16 port);

extern u08 pamela_find_slot(pamela_service_t *srv);

extern void pamela_set_status(pamela_channel_t *chn, u08 status);

extern void pamela_set_error(pamela_channel_t *chn, u08 error);

extern void pamela_set_open_error(pamela_channel_t *chn, u08 error);

extern void pamela_open_work(pamela_channel_t *chn, pamela_handler_ptr_t handler, u08 state);

extern void pamela_close_work(pamela_channel_t *chn, pamela_handler_ptr_t handler, u08 state);

extern void pamela_reset_work(pamela_channel_t *chn, pamela_handler_ptr_t handler, u08 state);

extern void pamela_read_pre_work(pamela_channel_t *chn, pamela_handler_ptr_t handler, u08 state);

extern void pamela_read_post_work(pamela_channel_t *chn, pamela_handler_ptr_t handler, u08 state);

extern void pamela_write_pre_work(pamela_channel_t *chn, pamela_handler_ptr_t handler, u08 state);

extern void pamela_write_post_work(pamela_channel_t *chn, pamela_handler_ptr_t handler, u08 state);

#endif
