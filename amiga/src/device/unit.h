#ifndef UNIT_H
#define UNIT_H

#include <exec/types.h>

#include "device.h"

struct DevUnitsBase
{
    struct DevBase  base;
    struct MinList  unitList;
};

struct DevUnit
{
    struct MinNode  node;
    ULONG           unitNum;
    ULONG           openCnt;
    struct DevUnitsBase *devBase;
};

/* API */
extern void UnitsInit(struct DevUnitsBase *devBase);
extern struct DevUnit *UnitOpen(struct IOStdReq *ior,
                                struct DevUnitsBase *devBase,
                                ULONG unitNum,
                                ULONG unitSize,
                                ULONG flags);
extern void UnitClose(struct IOStdReq *ior,
                      struct DevUnitsBase *devBase);
extern void UnitBeginIO(struct IOStdReq *ior,
                        struct DevUnitsBase *devBase);
extern LONG UnitAbortIO(struct IOStdReq *ior,
                        struct DevUnitsBase *devBase);

/* callbacks: unit interface */
extern BOOL UserUnitInit(struct DevUnit *unit, ULONG flags);
extern BOOL UserUnitOpen(struct IOStdReq *ior,
                         struct DevUnit *unit, ULONG flags);
extern void UserUnitClose(struct IOStdReq *ior,
                          struct DevUnit *unit);
extern void UserUnitExit(struct DevUnit *unit);

#endif