#include <proto/exec.h>

#include "autoconf.h"

#ifdef CONFIG_DEBUG_PAMELA_DEV
#define KDEBUG
#endif

#include "debug.h"

#include "device.h"
#include "unit.h"
#include "worker.h"
#include "pamela_dev.h"
#include "devices/pamela.h"
#include "pamela_engine.h"
#include "pamela/error.h"

DECLARE_DEVICE_VECTORS()
DECLARE_DEVICE("pamela.device", 1, 0, "07.07.2019", struct PamelaDev)

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
    D(("pamdev: invalid unit!\n"));
    return NULL;
  }

    /* open unit */
  struct PamelaUnit *unit = (struct PamelaUnit *)UnitOpen(ior,
    (struct DevUnitsBase *)devBase, unitNum, sizeof(struct PamelaUnit),
    0);
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
  pamela_req_t *req = (pamela_req_t *)ior;
  D(("pamdev: begin io\n"));
  pamela_engine_post_request(unit->engine, req);
  D(("pamdev: begin io done\n"));
}

LIBFUNC LONG DevAbortIO(REG(a1, struct IOStdReq *ior),
  REG(a6, struct DevBase *base))
{
  struct PamelaUnit *unit = (struct PamelaUnit *)ior->io_Unit;
  pamela_req_t *req = (pamela_req_t *)ior;
  D(("pamdev: abort io\n"));
  pamela_engine_cancel_request(unit->engine, req);
  D(("pamdev: abort io result=%ld\n", req->iopam_Req.io_Error));
  return req->iopam_Req.io_Error;
}

/* ----- unit functions ----- */

static BOOL worker_init(APTR user_data)
{
  struct PamelaUnit *unit = (struct PamelaUnit *)user_data;
  int error = 0;
  struct Library *sysBase = unit->unit.devBase->base.sysBase;
  D(("pamdev: wrk init\n"));
  unit->engine = pamela_engine_init(sysBase, &error);
  D(("pamdev: wrk init res=%ld\n", error));
  return error == PAMELA_OK;
}

static void worker_main(APTR user_data)
{
  struct PamelaUnit *unit = (struct PamelaUnit *)user_data;
  // main loop
  D(("pamdev: wrk main\n"));
  pamela_engine_work(unit->engine, 0);
  D(("pamdev: wrk exit\n"));
  pamela_engine_exit(unit->engine);
  D(("pamdev: wrk done\n"));
}

BOOL UserUnitInit(struct DevUnit *dev_unit, ULONG flags)
{
  struct PamelaUnit *unit = (struct PamelaUnit *)dev_unit;
  struct Library *sysBase = dev_unit->devBase->base.sysBase;

  // launch worker task with engine
  D(("pamdev: unit init\n"));
  unit->task = worker_run(sysBase, "pamdev", 8192,
    worker_init, worker_main, unit,
    &unit->quit_signal);
  D(("pamdev: unit init done: task=%lx\n", unit->task));
  return (unit->task != NULL);
}

void UserUnitExit(struct DevUnit *dev_unit)
{
  struct PamelaUnit *unit = (struct PamelaUnit *)dev_unit;
  struct Library *sysBase = dev_unit->devBase->base.sysBase;

  D(("pamdev: unit exit\n"));
  pamela_engine_quit(unit->engine);
  worker_join(sysBase, unit->quit_signal);
  D(("pamdev: unit exit done\n"));
}

BOOL UserUnitOpen(struct IOStdReq *ior,
  struct DevUnit *dev_unit,
  ULONG flags)
{
  struct PamelaUnit *unit = (struct PamelaUnit *)dev_unit;
  pamela_req_t *req = (pamela_req_t *)ior;
  D(("pamdev: unit open\n"));
  int result = pamela_engine_init_request(unit->engine, req);
  D(("pamdev: unit open done: %ld\n", result));
  return result == PAMELA_OK;
}

void UserUnitClose(struct IOStdReq *ior,
 struct DevUnit *dev_unit)
{
  struct PamelaUnit *unit = (struct PamelaUnit *)dev_unit;
  pamela_req_t *req = (pamela_req_t *)ior;
  D(("pamdev: unit close\n"));
  pamela_engine_exit_request(unit->engine, req);
  D(("pamdev: unit close done\n"));
}
