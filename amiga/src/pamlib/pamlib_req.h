#ifndef PAMLIB_REQ_H
#define PAMLIB_REQ_H

#include "pamlib.h"

struct pamlib_req;
typedef struct pamlib_req pamlib_req_t;

pamlib_req_t *pamlib_req_open(pamlib_handle_t *ch, UWORD port, int *error);
int pamlib_req_close(pamlib_req_t *req);

/* return max size of req/reply buffers */
UWORD pamlib_req_get_max_size(pamlib_req_t *req);

/* handle a request: pass in buffer with initial request data and size.
   will return reply data in same buffer and the resulting size as return value */
int pamlib_req_transfer(pamlib_req_t *req, UBYTE *buf, UWORD req_size);

#endif
