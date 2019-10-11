#include <proto/exec.h>

#include "device.h"
#include "pamela_dev.h"
#include "pamnet.h"

DECLARE_DEVICE_VECTORS()
DECLARE_DEVICE("pamnet.device", 1, 0, "07.07.2019", struct PamelaDev)

struct PamEngine *pamdev_engine_create(void)
{
    return pamnet_engine_create((struct Library *)SysBase);
}

void pamdev_engine_delete(struct PamEngine *pe)
{
    pamnet_engine_delete(pe);
}
