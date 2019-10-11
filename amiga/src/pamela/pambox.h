#ifndef PAMBOX_H
#define PAMBOX_H

#include "pam_engine.h"
#include "pamela.h"

struct PamBoxEngine {
    struct PamEngine  pbe_Engine;
    pamela_handle_t  *pbe_Handle;
};

struct PamEngine *pambox_engine_create(struct Library *sys_base);
void pambox_engine_delete(struct PamEngine *pe);

#endif
