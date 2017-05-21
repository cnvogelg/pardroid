#ifndef ENGINE_H
#define ENGINE_H

#include "proto.h"
#include "request.h"

struct engine_handle;
typedef struct engine_handle engine_handle_t;

extern engine_handle_t *engine_start(int *result, struct Library *sys_base);
extern void engine_stop(engine_handle_t *eh);

extern int engine_open_channel(engine_handle_t *eh, UBYTE channel);
extern int engine_close_channel(engine_handler_t *eh, UBYTE channel);

extern int engine_send_request(engine_handle_t *eh, request_t *r);

#endif
