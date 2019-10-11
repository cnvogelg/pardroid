#define __NOLIBBASE__
#include <proto/exec.h>
#include <exec/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "pam_engine.h"
#include "pamela.h"

struct PrivPamEngine {
    struct PamEngine  ppe_Engine;
    pamela_handle_t  *ppe_Handle;
};

static BOOL engine_init(struct PamEngine *pe, ULONG flags);
static void engine_exit(struct PamEngine *pe);

struct PamEngine *pam_engine_create(struct Library *SysBase)
{
    struct PrivPamEngine *ppe = (struct PrivPamEngine *)
        AllocVec(sizeof(struct PrivPamEngine),
                 MEMF_CLEAR | MEMF_PUBLIC);
    if(ppe == NULL) {
        return NULL;
    }

    struct PamEngine *pe = &ppe->ppe_Engine;
    pe->pe_SysBase = SysBase;
    pe->pe_InitFunc = engine_init;
    pe->pe_ExitFunc = engine_exit; 

    return (struct PamEngine *)ppe;
}

#undef SysBase
#define SysBase pe->pe_SysBase

BOOL pam_engine_init(struct PamEngine *pe, ULONG flags)
{
    if(pe->pe_InitFunc != NULL) {
        return pe->pe_InitFunc(pe, flags);
    } else {
        return FALSE;
    }
}

void pam_engine_exit(struct PamEngine *pe)
{
    if(pe->pe_ExitFunc != NULL) {
        pe->pe_ExitFunc(pe);
    }
}

void pam_engine_delete(struct PamEngine *pe)
{
    struct PrivPamEngine *ppe = (struct PrivPamEngine *)pe;

    FreeVec(ppe);
}

BOOL pam_engine_open(struct PamEngine *pe, struct IOPamReq *req)
{
    return TRUE;
}

void pam_engine_close(struct PamEngine *pe, struct IOPamReq *req)
{

}

void pam_engine_handle_signals(struct PamEngine *pe, ULONG signals)
{

}

void pam_engine_begin_io(struct PamEngine *pe, struct IOPamReq *req)
{

}

static BOOL engine_init(struct PamEngine *pe, ULONG flags)
{
    return TRUE;
}

static void engine_exit(struct PamEngine *pe)
{
}