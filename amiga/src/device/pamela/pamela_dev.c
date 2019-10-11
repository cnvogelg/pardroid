#include <proto/exec.h>

#include "autoconf.h"

#ifdef CONFIG_DEBUG_PAMELA_DEV
#define KDEBUG
#endif

#include "device.h"
#include "unit.h"
#include "devworker.h"
#include "debug.h"
#include "pamela_dev.h"
#include "pamela_worker.h"
#include "pam_engine.h"
#include "devices/pamela.h"

DECLARE_DEVICE_VECTORS()
DECLARE_DEVICE("pamela.device", 42, 37, "07.07.2019", struct PamelaDev)

struct DevBase *UserDevInit(struct DevBase *devBase)
{
    UnitsInit((struct DevUnitsBase *)devBase);
    return devBase;
}

void UserDevExit(struct DevBase *devBase)
{
}

struct DevBase *UserDevOpen(struct IOStdReq *ior, ULONG unitNum, ULONG flags,
                            struct DevBase *devBase)
{
    struct PamelaDev *dev = (struct PamelaDev *)devBase;

    /* check unit */
    if(unitNum>0) {
        D(("Pamela: invalid unit!\n"));
        return NULL;
    }

    /* bootloader mode? only allowed for the first user! */
    ULONG engineFlags = 0;
    if((flags & PAMOF_BOOTLOADER) == PAMOF_BOOTLOADER) {
        if(devBase->libBase.lib_OpenCnt > 0) {
            D(("Pamela: bootloader only for single user!\n"));
            return NULL;
        }
        engineFlags = PAMENG_FLAG_BOOTLOADER;
    }

    /* open unit */
    struct PamelaUnit *unit = (struct PamelaUnit *)UnitOpen(ior,
        (struct DevUnitsBase *)devBase, unitNum, sizeof(struct PamelaUnit),
        engineFlags);
    if(unit != NULL) {
        return devBase;
    } else {
        return NULL;
    }
}

void UserDevClose(struct IOStdReq *ior, struct DevBase *devBase)
{
    UnitClose(ior, (struct DevUnitsBase *)devBase);
}

LIBFUNC void DevBeginIO(REG(a1, struct IOStdReq *ior),
                        REG(a6, struct DevBase *base))
{
    struct PamelaUnit *unit = (struct PamelaUnit *)ior->io_Unit;
    DevWorkerBeginIO(&unit->worker, ior);
}

LIBFUNC LONG DevAbortIO(REG(a1, struct IOStdReq *ior),
                        REG(a6, struct DevBase *base))
{
    struct PamelaUnit *unit = (struct PamelaUnit *)ior->io_Unit;
    return DevWorkerAbortIO(&unit->worker, ior);
}

/* ----- unif functions ----- */

BOOL UserUnitInit(struct DevUnit *unit, ULONG flags)
{
    struct PamelaUnit *pbUnit = (struct PamelaUnit *)unit;
    struct DevWorker *worker = &pbUnit->worker;

    worker->userData = unit;
    worker->initFunc = pamela_worker_init;
    worker->exitFunc = pamela_worker_exit;
    worker->openFunc = pamela_worker_open;
    worker->closeFunc = pamela_worker_close;
    worker->beginIOFunc = pamela_worker_begin_io;
    worker->abortIOFunc = pamela_worker_abort_io;
    worker->sigFunc = pamela_worker_sig_func;

    pbUnit->engineFlags = flags;

    if(!DevWorkerStart(worker, "Pamela_dev.task", unit)) {
        return FALSE;
    }
    return TRUE;
}

void UserUnitExit(struct DevUnit *unit)
{
    struct PamelaUnit *pbUnit = (struct PamelaUnit *)unit;
    struct DevWorker *worker = &pbUnit->worker;

    DevWorkerStop(worker);
}

BOOL UserUnitOpen(struct IOStdReq *ior,
                  struct DevUnit *unit,
                  ULONG flags)
{
    struct PamelaUnit *pbUnit = (struct PamelaUnit *)unit;
    struct DevWorker *worker = &pbUnit->worker;

    return DevWorkerOpen(worker, ior, flags);
}

void UserUnitClose(struct IOStdReq *ior,
                   struct DevUnit *unit)
{
    struct PamelaUnit *pbUnit = (struct PamelaUnit *)unit;
    struct DevWorker *worker = &pbUnit->worker;

    DevWorkerClose(worker, ior);
}


