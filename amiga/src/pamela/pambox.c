#define __NOLIBBASE__
#include <proto/exec.h>
#include <exec/exec.h>

#include "autoconf.h"

#ifdef CONFIG_DEBUG_PAM_BOX
#define KDEBUG
#endif

#include "pambox.h"
#include "debug.h"

static BOOL engine_init(struct PamEngine *pe, ULONG flags);
static void engine_exit(struct PamEngine *pe);
static BOOL engine_open(struct PamEngine *pe, struct IOPamReq *req);
static void engine_close(struct PamEngine *pe, struct IOPamReq *req);
static void engine_handle_signals(struct PamEngine *pe, ULONG signals);
static BOOL engine_begin_io(struct PamEngine *pe, struct IOPamReq *req);
static BOOL engine_abort_io(struct PamEngine *pe, struct IOPamReq *req);

struct PamEngine *pambox_engine_create(struct Library *SysBase)
{
    struct PamBoxEngine *pbe = (struct PamBoxEngine *)
        AllocVec(sizeof(struct PamBoxEngine),
                 MEMF_CLEAR | MEMF_PUBLIC);
    if(pbe == NULL) {
        return NULL;
    }

    struct PamEngine *pe = &pbe->pbe_Engine;
    pe->pe_SysBase = SysBase;
    pe->pe_InitFunc = engine_init;
    pe->pe_ExitFunc = engine_exit; 
    pe->pe_OpenFunc = engine_open;
    pe->pe_CloseFunc = engine_close;
    pe->pe_HandleSignalsFunc = engine_handle_signals;
    pe->pe_BeginIOFunc = engine_begin_io;
    pe->pe_AbortIOFunc = engine_abort_io;

    return pe;
}

#undef SysBase
#define SysBase pe->pe_SysBase

void pambox_engine_delete(struct PamEngine *pe)
{
    FreeVec(pe);
}

static BOOL engine_init(struct PamEngine *pe, ULONG flags)
{
    D(("pambox: init\n"));
    return TRUE;
}

static void engine_exit(struct PamEngine *pe)
{
    D(("pambox: exit\n"));
}

static BOOL engine_open(struct PamEngine *pe, struct IOPamReq *req)
{
    D(("pambox: open\n"));
    return TRUE;
}

static void engine_close(struct PamEngine *pe, struct IOPamReq *req)
{
    D(("pambox: close\n"));
}

static void engine_handle_signals(struct PamEngine *pe, ULONG signals)
{
    D(("pambox: handle signals\n"));
}

static BOOL engine_begin_io(struct PamEngine *pe, struct IOPamReq *req)
{
    D(("pambox: begin IO\n"));
    return TRUE;
}

static BOOL engine_abort_io(struct PamEngine *pe, struct IOPamReq *req)
{
    D(("pambox: abort IO\n"));
    return TRUE;
}
