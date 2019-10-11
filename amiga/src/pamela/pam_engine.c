#define __NOLIBBASE__
#include <proto/exec.h>
#include <exec/exec.h>

#include "autoconf.h"

#ifdef CONFIG_DEBUG_PAM_ENGINE
#define KDEBUG
#endif

#include "compiler.h"
#include "debug.h"

#include "pam_engine.h"

#undef SysBase
#define SysBase pe->pe_SysBase

BOOL pam_engine_init(struct PamEngine *pe, ULONG flags)
{
    if(pe->pe_InitFunc != NULL) {
        D(("+Engine: init\n"));
        BOOL result = pe->pe_InitFunc(pe, flags);
        D(("-Engine: init %ld\n", result));
        return result;
    } else {
        D(("Engine: no init!\n"));
        return FALSE;
    }
}

void pam_engine_exit(struct PamEngine *pe)
{
    if(pe->pe_ExitFunc != NULL) {
        D(("+Engine: exit\n"));
        pe->pe_ExitFunc(pe);
        D(("-Engine: exit\n"));
    }
}

BOOL pam_engine_open(struct PamEngine *pe, struct IOPamReq *req)
{
    if(pe->pe_OpenFunc != NULL) {
        D(("+Engine: init\n"));
        BOOL result = pe->pe_OpenFunc(pe, req);
        D(("+Engine: init\n"));
        return result;
    } else {
        D(("Engine: no open!\n"));
        return FALSE;
    }
}

void pam_engine_close(struct PamEngine *pe, struct IOPamReq *req)
{
    if(pe->pe_CloseFunc != NULL) {
        D(("+Engine: close\n");)
        pe->pe_CloseFunc(pe, req);
        D(("-Engine: close\n");)
    } else {
        D(("Engine: no close!\n"));
    }
}

void pam_engine_handle_signals(struct PamEngine *pe, ULONG signals)
{
    if(pe->pe_HandleSignalsFunc != NULL) {
        D(("+Engine: handle signals: %08lx\n", signals);)
        pe->pe_HandleSignalsFunc(pe, signals);
        D(("-Engine: handle signals\n"));
    } else {
        D(("Engine: no handle signals!\n"));
    }
}

BOOL pam_engine_begin_io(struct PamEngine *pe, struct IOPamReq *req)
{
    if(pe->pe_BeginIOFunc != NULL) {
        D(("+Engine: begin io\n"));
        BOOL result = pe->pe_BeginIOFunc(pe, req);
        D(("-Engine: begin io %ld\n", result));
        return result;
    } else {
        D(("Engine: no begin io!\n"));
        return FALSE;
    }
}

BOOL pam_engine_abort_io(struct PamEngine *pe, struct IOPamReq *req)
{
    if(pe->pe_AbortIOFunc != NULL) {
        D(("+Engine: abort io\n"));
        BOOL result = pe->pe_AbortIOFunc(pe, req);
        D(("-Engine: begin io %ld\n", result));
        return result;
    } else {
        D(("Engine: no abort io!\n"));
        return FALSE;
    }
}
