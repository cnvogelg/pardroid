#ifndef ENGINE_H
#define ENGINE_H

#include "proto.h"
#include "request.h"

#define ENGINE_RET_MASK              0xf0

#define ENGINE_RET_OK                0x00
#define ENGINE_RET_INIT_FAILED       0x10
#define ENGINE_RET_OPEN_ERROR        0x20
#define ENGINE_RET_CLOSE_ERROR       0x30
#define ENGINE_RET_OUT_OF_MEMORY     0x40

struct engine_handle;
typedef struct engine_handle engine_handle_t;

extern engine_handle_t *engine_start(int *result, struct Library *sys_base);
extern void engine_stop(engine_handle_t *eh);

extern int engine_open_channel(engine_handle_t *eh, UBYTE channel);
extern int engine_close_channel(engine_handle_t *eh, UBYTE channel);
extern int engine_close_all_channels(engine_handle_t *eh);

extern void engine_send_request(engine_handle_t *eh, request_t *r);

extern const char *engine_perror(int res);

#endif
