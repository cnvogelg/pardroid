#ifndef PAMELA_DEV_H
#define PAMELA_DEV_H

#include "unit.h"
#include "pamela_engine.h"

struct PamelaDev {
    struct DevUnitsBase devBase;
};

struct PamelaUnit {
    struct DevUnit    unit;
    struct Task      *task;
    pamela_engine_t  *engine;
    BYTE              quit_signal;
};

#endif
