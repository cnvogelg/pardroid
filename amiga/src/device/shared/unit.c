#include <exec/types.h>
#include <proto/exec.h>
#include <proto/alib.h>

#include "autoconf.h"

#ifdef CONFIG_DEBUG_DEVICE
#define KDEBUG
#endif

#include "unit.h"
#include "debug.h"

void UnitsInit(struct DevUnitsBase *devBase)
{
    NewList((struct List *)&devBase->unitList);
}

struct DevUnit *UnitFind(struct DevUnitsBase *devBase,
                         ULONG unitNum)
{
    struct MinNode *node = devBase->unitList.mlh_Head;
    struct MinNode *end = (struct MinNode *)&devBase->unitList.mlh_Tail;

    while(node != end) {
        struct DevUnit *unit = (struct DevUnit *)node;
        if(unit->unitNum == unitNum) {
            return unit;
        }
        node = node->mln_Succ;
    }
    return NULL;
}

struct DevUnit *UnitOpen(struct IOStdReq *ior,
                         struct DevUnitsBase *devBase,
                         ULONG unitNum,
                         ULONG unitSize)
{
    struct DevUnit *unit;

    D(("+UnitOpen(%08lx, %08lx, %ld, %ld)\n", ior, devBase, unitNum, unitSize));
    ior->io_Unit = NULL;

    /* already got unit? */
    unit = UnitFind(devBase, unitNum);
    /* no, create new unit */
    if(unit == NULL) {
        /* alloc unit */
        unit = (struct DevUnit *)AllocVec(unitSize, MEMF_PUBLIC);
        if(unit == NULL) {
            D(("-UnitOpen: no mem!\n"));
            return NULL;
        }
        /* local init */
        unit->unitNum = unitNum;
        unit->openCnt = 0;
        unit->devBase = devBase;
        /* setup unit */
        if(!UserUnitInit(unit)) {
            FreeVec(unit);
            D(("-UnitOpen: UserUnitInit failed!\n"));
            return NULL;
        }
        /* add to unit list */
        AddHead((struct List *)&devBase->unitList, (struct Node *)unit);
    }

    /* check if open is ok */
    if(!UserUnitOpen(ior, unit)) {
        /* if no users left -> remove it now */
        if(unit->openCnt==0) {
            UserUnitExit(unit);
            Remove((struct Node *)unit);
            FreeVec(unit);
            D(("-UnitOpen: UserUnitOpen failed!\n"));
            return NULL;
        }
    }

    unit->openCnt++;
    ior->io_Unit = (struct Unit *)unit;

    D(("-UnitOpen: %08lx\n", unit));
    return unit;
}

void UnitClose(struct IOStdReq *ior,
               struct DevUnitsBase *devBase)
{
    struct DevUnit *unit = (struct DevUnit *)ior->io_Unit;

    D(("+UnitClose(%08lx, %08lx): %08lx\n", ior, devBase, unit));
    UserUnitClose(ior, unit);
    unit->openCnt--;

    if(unit->openCnt==0) {
        UserUnitExit(unit);
        Remove((struct Node *)unit);
        FreeVec(unit);
    }

    ior->io_Unit = NULL;

    D(("-UnitClose\n"));
}
