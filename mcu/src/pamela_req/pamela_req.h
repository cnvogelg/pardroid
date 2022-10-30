#ifndef PAMELA_REQ_H
#define PAMELA_REQ_H

#include "pamela.h"
#include "pamela_req_handler.h"
#include "pamela_req_int.h"

#define PAMELA_REQ_NO_SERVICE_ID 0xff

extern u08 pamela_req_add_handler(pamela_handler_ptr_t pam_hnd,
                                  pamela_req_handler_ptr_t req_hnd,
                                  pamela_req_slot_t *slots);

/* helpers for req implementation */
extern u08 pamela_req_open_malloc(u08 chan, pamela_buf_t *buf);
extern void pamela_req_close_free(u08 chan, pamela_buf_t *buf);

#endif
