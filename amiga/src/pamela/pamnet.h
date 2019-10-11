#ifndef PAMNET_H
#define PAMNET_H

#include "pam_engine.h"

struct PamNetEngine {
    struct PamEngine  pne_Engine;
};

struct PamEngine *pamnet_engine_create(struct Library *sys_base);
void pamnet_engine_delete(struct PamEngine *pe);

#endif
