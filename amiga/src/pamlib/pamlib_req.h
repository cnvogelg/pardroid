#ifndef PAMLIB_REQ_H
#define PAMLIB_REQ_H

#include "pamlib.h"

struct pamlib_req {
  UWORD  tx_size;
  UWORD  rx_size;
  UWORD  mtu;
  UBYTE *buf;
};
typedef struct pamlib_req pamlib_req_t;

pamlib_req_t *pamlib_req_open(pamlib_handle_t *ch, UWORD port, int *error);
int pamlib_req_close(pamlib_req_t *req);

int pamlib_req_transfer(pamlib_req_t *req);

#endif
