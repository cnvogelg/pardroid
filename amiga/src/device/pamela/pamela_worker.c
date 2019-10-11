#include <proto/exec.h>
#include <exec/types.h>

#include "autoconf.h"

#ifdef CONFIG_DEBUG_PAMELA_DEV
#define KDEBUG
#endif

#include "debug.h"
#include "pam_engine.h"
#include "pamela_dev.h"
#include "pamela_worker.h"

BOOL pamela_worker_init(struct DevWorker *worker)
{
    D(("Pamela: worker init\n"));

    /* create and setup pamela engine */
    struct PamEngine *engine = pam_engine_create((struct Library *)SysBase);
    if(engine == NULL) {
        D(("ERROR: no pam engine!\n"));
        return FALSE;
    }

    /* init engine */
    struct PamelaUnit *unit = (struct PamelaUnit *)worker->userData;
    if(!pam_engine_init(engine, unit->engineFlags)) {
        D(("ERROR: pam init failed. %ld\n", engine->pe_Error));
        return FALSE;
    }
    unit->engine = engine;

    /* setup worker */
    worker->extraSigMask = engine->pe_SigMask;

    D(("Pamela: init OK!\n"));
    return TRUE;
}

void pamela_worker_exit(struct DevWorker *worker)
{
    D(("Pamela: worker exit\n"));

    struct PamelaUnit *unit = (struct PamelaUnit *)worker->userData;
    if(unit->engine != NULL) {
        pam_engine_exit(unit->engine);
        pam_engine_delete(unit->engine);
        unit->engine = NULL;
    }

    D(("Pamela: exit OK!\n"));
}

void pamela_worker_sig_func(struct DevWorker *worker, ULONG mask)
{
    D(("Pamela: worker sig mask=%08lx\n", mask));
}

BOOL pamela_worker_open(struct DevWorker *worker, struct IOStdReq *ior, ULONG flags)
{
    D(("Pamela: open\n"));
    return TRUE;
}

void pamela_worker_close(struct DevWorker *worker, struct IOStdReq *ior)
{
    D(("Pamela: close\n"));
}

BOOL pamela_worker_begin_io(struct DevWorker *worker, struct IOStdReq *ior)
{
    D(("Pamela: begin_io: cmd=%08lx\n", ior->io_Command));
    return TRUE;
}

BOOL pamela_worker_abort_io(struct DevWorker *worker, struct IOStdReq *ior)
{
    D(("Pamela: abort_io: cmd=%08lx\n", ior->io_Command));
    return TRUE;
}
