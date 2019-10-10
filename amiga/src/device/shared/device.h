#ifndef DEVICE_H
#define DEVICE_H

#include <exec/exec.h>
#include <dos/dos.h>
#include "compiler.h"

#define LIBFUNC SAVEDS ASM

struct DevBase {
    struct Library          libBase;
    struct Library *        sysBase;
    ULONG                   segList;
    struct SignalSemaphore  devSem;
};

LIBFUNC extern struct DevBase * DevInit    (REG(a0, BPTR Segment),
                                            REG(d0, struct DevBase *lh),
                                            REG(a6, struct ExecBase *sb));
LIBFUNC extern BPTR             DevExpunge (REG(a6, struct DevBase *base));
LIBFUNC extern struct DevBase * DevOpen    (REG(a1, struct IOStdReq *ior),
                                            REG(d0, ULONG unit),
                                            REG(d1, ULONG flags),
                                            REG(a6, struct DevBase *base));
LIBFUNC extern BPTR             DevClose   (REG(a1, struct IOStdReq *ior),
                                            REG(a6, struct DevBase *base));
LIBFUNC extern LONG             DevNull    (void);
LIBFUNC extern void             DevBeginIO(REG(a1, struct IOStdReq *ior),
                                           REG(a6, struct DevBase *base));
LIBFUNC extern LONG             DevAbortIO(REG(a1, struct IOStdReq *ior),
                                           REG(a6, struct DevBase *base));

#define DECLARE_DEVICE(name, ver_maj, ver_min, date, dev_struct) \
int entry(void) { return RETURN_FAIL; } \
const char UserLibName[]; \
const char UserLibVer[]; \
static const ULONG LibInitTab[]; \
static const struct Resident ROMTag = \
{ \
  RTC_MATCHWORD, \
  (struct Resident *)&ROMTag, \
  (struct Resident *)&ROMTag + 1, \
  RTF_AUTOINIT, \
  ver_maj, \
  NT_DEVICE, \
  0, /* prio */ \
  (APTR)UserLibName, \
  (APTR)UserLibVer, \
  (APTR)LibInitTab \
}; \
const char UserLibName[] = name; \
const char UserLibVer[]  = name " " #ver_maj "." #ver_min " (" date ")\r\n"; \
const char UserLibID[]   = "\0$VER: " name " " #ver_maj "." #ver_min " (" date ")"; \
const UWORD UserLibVerMaj = ver_maj; \
const UWORD UserLibVerMin = ver_min; \
static const ULONG LibInitTab[] = \
{ \
  sizeof(dev_struct), \
  (ULONG)LibVectors, \
  (ULONG)NULL, \
  (ULONG)DevInit \
}; \

#define DECLARE_DEVICE_VECTORS(extra) \
static const APTR LibVectors[] = \
{ \
  (APTR)DevOpen, \
  (APTR)DevClose, \
  (APTR)DevExpunge, \
  (APTR)DevNull, \
  (APTR)DevBeginIO, \
  (APTR)DevAbortIO, \
  extra \
  (APTR)-1 \
};

/* callbacks: device interface */
extern struct DevBase *UserDevInit(struct DevBase *devBase);
extern void UserDevExit(struct DevBase *devBase);
extern struct DevBase *UserDevOpen(struct IOStdReq *ior, ULONG unit, ULONG flags,
                                   struct DevBase *devBase);
extern void UserDevClose(struct IOStdReq *ior, struct DevBase *devBase);

#endif