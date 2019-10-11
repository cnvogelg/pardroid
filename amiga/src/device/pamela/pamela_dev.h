#ifndef PAMELA_DEV_H
#define PAMELA_DEV_H

#include "unit.h"
#include "devworker.h"
#include "pam_engine.h"

struct PamelaDev {
    struct DevUnitsBase devBase;
};

struct PamelaUnit {
    struct Unit       unit;
    struct DevWorker  worker;
    struct PamEngine *engine;
    ULONG             engineFlags;
};

/* API per device class */
extern struct PamEngine *pamdev_engine_create(void);
extern void pamdev_engine_delete(struct PamEngine *pe);

#endif
