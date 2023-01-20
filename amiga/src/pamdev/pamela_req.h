#ifndef PAMELA_REQ_H
#define PAMELA_REQ_H

void pamela_req_handle(pamela_engine_t *eng, pamela_req_t *req);

#define pamela_req_get_client(req) ((pamela_client_t *)req->iopam_Internal)
#define pamela_req_get_channel(req) (req->iopam_Channel)

#endif
