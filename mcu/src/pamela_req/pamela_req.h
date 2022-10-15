#ifndef PAMELA_REQ_H
#define PAMELA_REQ_H

#include "pamela.h"
#include "pamela_req_handler.h"

// total number of command handlers
#ifndef PAMELA_REQ_NUM_HANDLERS
#define PAMELA_REQ_NUM_HANDLERS   4
#endif

#define PAMELA_REQ_NO_SERVICE_ID 0xff

extern u08 pamela_req_add_handler(pamela_handler_ptr_t pam_hnd,
                                  pamela_req_handler_ptr_t req_hnd,
                                  pamela_req_slot_t *slots);

#endif
