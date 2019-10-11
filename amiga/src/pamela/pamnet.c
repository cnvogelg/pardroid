#define __NOLIBBASE__
#include <proto/exec.h>
#include <exec/exec.h>

#include "autoconf.h"

#ifdef CONFIG_DEBUG_PAM_NET
#define KDEBUG
#endif

#include "pamnet.h"
#include "debug.h"

static BOOL engine_init(struct PamEngine *pe, ULONG flags);
static void engine_exit(struct PamEngine *pe);

struct PamEngine *pamnet_engine_create(struct Library *SysBase)
{
    struct PamNetEngine *pne = (struct PamNetEngine *)
        AllocVec(sizeof(struct PamNetEngine),
                 MEMF_CLEAR | MEMF_PUBLIC);
    if(pne == NULL) {
        return NULL;
    }

    struct PamEngine *pe = &pne->pne_Engine;
    pe->pe_SysBase = SysBase;
    pe->pe_InitFunc = engine_init;
    pe->pe_ExitFunc = engine_exit; 

    return pe;
}

#undef SysBase
#define SysBase pe->pe_SysBase

void pamnet_engine_delete(struct PamEngine *pe)
{
    FreeVec(pe);
}

static BOOL engine_init(struct PamEngine *pe, ULONG flags)
{
    D(("pamnet: init\n"));
    return TRUE;
}

static void engine_exit(struct PamEngine *pe)
{
    D(("pamnet: exit\n"));
}
