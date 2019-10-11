#ifndef PAM_ENGINE_H
#define PAM_ENGINE_H

#include <exec/types.h>
#include "devices/pamela.h"

#define PAMENG_FLAG_BOOTLOADER       1

struct PamEngine;

typedef BOOL (*PamEngineInitFunc)(struct PamEngine *e, ULONG flags);
typedef void (*PamEngineExitFunc)(struct PamEngine *e);
typedef BOOL (*PamEngineOpenFunc)(struct PamEngine *e, struct IOPamReq *req);
typedef void (*PamEngineCloseFunc)(struct PamEngine *e, struct IOPamReq *req);
typedef void (*PamEngineHandleSignalsFunc)(struct PamEngine *pe, ULONG signals);
typedef BOOL (*PamEngineIOFunc)(struct PamEngine *pe, struct IOPamReq *req);

struct PamEngine {
    /* engine methods */
    PamEngineInitFunc   pe_InitFunc;
    PamEngineExitFunc   pe_ExitFunc;
    PamEngineOpenFunc   pe_OpenFunc;
    PamEngineCloseFunc  pe_CloseFunc;
    PamEngineHandleSignalsFunc pe_HandleSignalsFunc;
    PamEngineIOFunc     pe_BeginIOFunc;
    PamEngineIOFunc     pe_AbortIOFunc;
    /* parameter */
    ULONG               pe_SigMask;
    struct Library     *pe_SysBase;
    /* state */
    UWORD               pe_OpenCnt;
    UWORD               pe_Error;
};

BOOL pam_engine_init(struct PamEngine *pe, ULONG flags);
void pam_engine_exit(struct PamEngine *pe);

BOOL pam_engine_open(struct PamEngine *pe, struct IOPamReq *req);
void pam_engine_close(struct PamEngine *pe, struct IOPamReq *req);

void pam_engine_handle_signals(struct PamEngine *pe, ULONG signals);

BOOL pam_engine_begin_io(struct PamEngine *pe, struct IOPamReq *req);
BOOL pam_engine_abort_io(struct PamEngine *pe, struct IOPamReq *req);

#endif
