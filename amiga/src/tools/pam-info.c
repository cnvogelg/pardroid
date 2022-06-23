#include <proto/exec.h>
#include <proto/dos.h>
#include <clib/alib_protos.h>

#include <dos/dos.h>
#include <exec/exec.h>

#include "autoconf.h"
#include "debug.h"

#include "pamlib.h"

static const char *TEMPLATE = "D=DEVICE/K";
typedef struct {
  char *device;
} params_t;
static params_t params;

int pam_info(pamlib_handle_t *ph)
{
    pamela_devinfo_t dev_info;

    int error = pamlib_devinfo(ph, &dev_info);
    if(error != PAMELA_OK) {
        Printf("Failed to read devinfo: %ld %s\n", error, (LONG)pamela_perror(error));
        return RETURN_ERROR;
    }

    /* print dev info */
    Printf("firmware id:      0x%04lx\n", dev_info.firmware_id);
    Printf("firmware version: 0x%04lx\n", dev_info.firmware_version);
    Printf("mach tag:         0x%04lx\n", dev_info.mach_tag);
    Printf("default MTU:      %ld\n", dev_info.default_mtu);
    Printf("max channels:     %ld\n", dev_info.max_channels);

    return RETURN_OK;
}

int dosmain(void)
{
    struct RDArgs *args;
    char *device = "pamela.device";
    pamlib_handle_t *ph;

    /* First parse args */
    args = ReadArgs(TEMPLATE, (LONG *)&params, NULL);
    if(args == NULL) {
        PrintFault(IoErr(), "Args Error");
        return RETURN_ERROR;
    }
    if(params.device) {
        device = params.device;
    }

    int result = 0;
    int error = 0;
    ph = pamlib_init((struct Library *)SysBase, &error, device);
    if(ph != NULL) {

        result = pam_info(ph);

        pamlib_exit(ph);
    } else {
        Printf("Can't Init pamlib: %ld %s\n", error, (LONG)pamela_perror(error));
        result = 1;
    }

    /* Finally free args */
    FreeArgs(args);
    return result;
}
