#include <exec/types.h>
#include <proto/exec.h>

#include "autoconf.h"

#ifdef CONFIG_DEBUG_DEVICE
#define KDEBUG
#endif

#include "compiler.h"
#include "debug.h"
#include "device.h"

extern const char UserLibName[];
extern const char UserLibVer[];
extern const char UserLibID[];
extern UWORD UserLibVerMin;
extern UWORD UserLibVerMaj;

#define DeleteLibrary(LIB) \
  FreeMem((STRPTR)(LIB)-(LIB)->lib_NegSize, (ULONG)((LIB)->lib_NegSize+(LIB)->lib_PosSize))

/* ----- Functions ----- */

LIBFUNC struct DevBase * DevInit(REG(a0, BPTR Segment),
                                 REG(d0, struct DevBase *base),
                                 REG(a6, struct ExecBase *sb))
{
  base->libBase.lib_Node.ln_Type = NT_LIBRARY;
  base->libBase.lib_Node.ln_Pri  = 0;
  base->libBase.lib_Node.ln_Name = (char *)UserLibName;
  base->libBase.lib_Flags        = LIBF_CHANGED | LIBF_SUMUSED;
  base->libBase.lib_Version      = UserLibVerMin;
  base->libBase.lib_Revision     = UserLibVerMaj;
  base->libBase.lib_IdString     = (char *)UserLibVer;

  base->segList = Segment;
  base->sysBase = (APTR)sb;

  InitSemaphore(&base->devSem);

  D(("+DevInit(%08lx, %08lx, %08lx)\n", Segment, base, sb));
  struct DevBase *result = UserDevInit(base);
  D(("-DevInit: result=%08lx\n", result));
  return result;
}

LIBFUNC BPTR DevExpunge(REG(a6, struct DevBase *base))
{
  BPTR rc;

  D(("+DevExpunge(%08lx)\n", base));

  if(base->libBase.lib_OpenCnt > 0)
  {
    base->libBase.lib_Flags |= LIBF_DELEXP;
    D(("-DevExpunge: DELEXP\n"));
    return 0;
  }

  UserDevExit(base);

  rc = base->segList;

  Remove((struct Node *)base);
  DeleteLibrary(&base->libBase);

  D(("-DevExpunge: %08lx\n", rc));
  return rc;
}

LIBFUNC struct DevBase * DevOpen(REG(a1, struct IOStdReq *ior),
                                 REG(d0, ULONG unit),
                                 REG(d1, ULONG flags),
                                 REG(a6, struct DevBase *base))
{
  D(("+DevOpen(%lx,%ld,%ld)\n", ior, unit, flags));
  if(base == NULL) {
    ior->io_Error = IOERR_OPENFAIL;
    return NULL;
  }

  /* take sem to ensure single task here - we may drop out of Forbid()! */
  ObtainSemaphore(&base->devSem);

  base->libBase.lib_OpenCnt++;

  struct DevBase *result = UserDevOpen(ior, unit, flags, base);

  if(result == NULL) {
    base->libBase.lib_OpenCnt--;
    ior->io_Error = IOERR_OPENFAIL;
    ior->io_Device = NULL;
  } else {
    base->libBase.lib_Flags &= ~LIBF_DELEXP;
    ior->io_Error = 0;
    ior->io_Device = (struct Device *)base;
  }

  ReleaseSemaphore(&base->devSem);

  D(("-DevOpen: result=%08lx\n", result));
  return result;
}

LIBFUNC BPTR DevClose(REG(a1, struct IOStdReq *ior),
                      REG(a6, struct DevBase *base))
{
  BPTR result = 0;
  D(("+DevClose(%08lx)\n", ior));

  /* take sem to ensure single task here - we may drop out of Forbid()! */
  ObtainSemaphore(&base->devSem);

  UserDevClose(ior, base);

  base->libBase.lib_OpenCnt--;
  if(base->libBase.lib_OpenCnt == 0)
  {
    if(base->libBase.lib_Flags & LIBF_DELEXP)
    {
      result = DevExpunge(base);
    }
  }

  // clear
  ior->io_Device = NULL;

  ReleaseSemaphore(&base->devSem);

  D(("-DevClose: result=%08lx\n", result));
  return 0;
}

LIBFUNC LONG DevNull(void)
{
  return 0;
}
