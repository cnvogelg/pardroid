#ifndef ENGINE_H
#define ENGINE_H

#include "proto.h"

struct engine_handle;
typedef struct engine_handle engine_handle_t;

extern engine_handle_t *engine_start(int *result, struct Library *sys_base);
extern void engine_stop(engine_handle_t *eh);

#endif
