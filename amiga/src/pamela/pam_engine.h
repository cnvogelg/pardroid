#ifndef PAM_ENGINE_H
#define PAM_ENGINE_H

#include <exec/types.h>
#include "devices/pamela.h"

#define PAMENG_FLAG_BOOTLOADER       1

struct PamEngine;

typedef BOOL (*PamEngineInitFunc)(struct PamEngine *e, ULONG flags);
typedef void (*PamEngineExitFunc)(struct PamEngine *e);

struct PamEngine {
    PamEngineInitFunc   pe_InitFunc;
    PamEngineExitFunc   pe_ExitFunc;
    ULONG               pe_SigMask;
    struct Library     *pe_SysBase;
    UWORD               pe_OpenCnt;
    UWORD               pe_Error;
};

struct PamEngine *pam_engine_create(struct Library *sys_base);
void pam_engine_delete(struct PamEngine *pe);

BOOL pam_engine_init(struct PamEngine *pe, ULONG flags);
void pam_engine_exit(struct PamEngine *pe);

BOOL pam_engine_open(struct PamEngine *pe, struct IOPamReq *req);
void pam_engine_close(struct PamEngine *pe, struct IOPamReq *req);

void pam_engine_handle_signals(struct PamEngine *pe, ULONG signals);
void pam_engine_begin_io(struct PamEngine *pe, struct IOPamReq *req);

#endif
