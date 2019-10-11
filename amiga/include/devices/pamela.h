#ifndef DEVICES_PAMELA_H
#define DEVICES_PAMELA_H

/*
**      Pamela - The parallel messsage layer
*/

#include <exec/types.h>
#include <exec/io.h>

/* IO Structure */
struct IOPamReq {
    struct IOStdReq     iopam_Req;
    UWORD               iopam_Channel; /* channel or channel mask */
    UWORD               iopam_ErrorDetail; /* detailed error code */ 
    APTR                iopam_Private; /* do not touch */
};

/* OpenDevice() Flags */
#define PAMOB_BOOTLOADER        0         /* enter bootloader */
#define PAMOF_BOOTLOADER        (1<<0)

/* Name of Device */
#define PAMELA_NAME             "pamela.device"

/* Custom Commands */
#define PAMCMD_DEVICEQUERY      (CMD_NONSTD)
#define PAMCMD_CHANNELQUERY     (CMD_NONSTD+1)

/* Error Codes */
#define PAMERR_NO_ERROR         0
#define PAMERR_PARIO            1
#define PAMERR_PROTO            2
#define PAMERR_TIMER            3

#endif
