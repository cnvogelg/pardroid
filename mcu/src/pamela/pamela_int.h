#ifndef PAMELA_INT_H
#define PAMELA_INT_H

#include "pamela.h"

struct pamela_service {
  /* the associated handler */
  pamela_handler_ptr_t handler;
  /* the associated channel mask */
  u16 channels;
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

  /* pending rx request */
  u16 rx_size;
  u08 *rx_buf;
  /* pending tx request */
  u16 tx_size;
  u08 *tx_buf;
};
typedef struct pamela_channel pamela_channel_t;

extern pamela_channel_t *pamela_get_channel(u08 chn);

extern pamela_service_t *pamela_find_service(u16 port);

#endif
