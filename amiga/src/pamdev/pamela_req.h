#ifndef PAMELA_REQ_H
#define PAMELA_REQ_H

int pamela_req_open(pamela_engine_t *eng, pamela_req_t *req);
int pamela_req_close(pamela_engine_t *eng, pamela_req_t *req);
int pamela_req_reset(pamela_engine_t *eng, pamela_req_t *req);

int pamela_req_seek(pamela_engine_t *eng, pamela_req_t *req);
int pamela_req_tell(pamela_engine_t *eng, pamela_req_t *req);

int pamela_req_read(pamela_engine_t *eng, pamela_req_t *req);
int pamela_req_write(pamela_engine_t *eng, pamela_req_t *req);

int pamela_req_devinfo(pamela_engine_t *eng, pamela_req_t *req);

#define pamela_req_get_client(req) ((pamela_client_t *)req->iopam_Internal)
#define pamela_req_get_channel(req) (req->iopam_Channel)

#endif
