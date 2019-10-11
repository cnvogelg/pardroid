#ifndef Pamela_DEV_H
#define Pamela_DEV_H

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

#endif
