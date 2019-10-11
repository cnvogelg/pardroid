
#include "autoconf.h"

#ifdef CONFIG_DEBUG_PAMELA_DEV
#define KDEBUG
#endif

#include "device.h"
#include "unit.h"
#include "devworker.h"
#include "debug.h"
#include "pamela_dev.h"

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
    /* check unit */
    if(unitNum>0) {
        return NULL;
    }

    /* open unit */
    struct PamelaUnit *unit = (struct PamelaUnit *)UnitOpen(ior,
        (struct DevUnitsBase *)devBase, unitNum, sizeof(struct PamelaUnit));
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
    DevWorkerBeginIO(ior, &unit->worker);
}

LIBFUNC LONG DevAbortIO(REG(a1, struct IOStdReq *ior),
                        REG(a6, struct DevBase *base))
{
    struct PamelaUnit *unit = (struct PamelaUnit *)ior->io_Unit;
    return DevWorkerAbortIO(ior, &unit->worker);
}

static BOOL workerInit(struct DevWorker *worker)
{
    D(("Pamela: worker init\n"));
    worker->extraSigMask = 0;

    return TRUE;
}

static void workerExit(struct DevWorker *worker)
{
    D(("Pamela: worker exit\n"));
}

static BOOL workerHandle(struct DevWorker *worker, struct IOStdReq *ior)
{
    D(("Pamela: handle: cmd=%08lx\n", ior->io_Command));
    return TRUE;
}

BOOL UserUnitInit(struct DevUnit *unit)
{
    struct PamelaUnit *pbUnit = (struct PamelaUnit *)unit;
    struct DevWorker *worker = &pbUnit->worker;

    worker->userData = unit;
    worker->initFunc = workerInit;
    worker->exitFunc = workerExit;
    worker->handlerFunc = workerHandle;

    if(!DevWorkerStart(worker, "Pamela_dev.task")) {
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
                 struct DevUnit *unit)
{
    return TRUE;
}

void UserUnitClose(struct IOStdReq *ior,
                   struct DevUnit *unit)
{
}


