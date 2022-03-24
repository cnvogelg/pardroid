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
    UBYTE               iopam_Channel; /* channel or channel mask */
    BYTE                iopam_PamelaError; /* detailed error code */
    UWORD               iopam_Port; /* port of service to connect */
    APTR                iopam_Internal; /* internal do not use and alter */
};

/* OpenDevice() Flags */
#define PAMOB_BOOTLOADER        0         /* enter bootloader */
#define PAMOF_BOOTLOADER        (1<<0)

/* Name of Device */
#define PAMELA_NAME             "pamela.device"

/* Custom Commands */
#define PAMCMD_OPEN_CHANNEL      (CMD_NONSTD+0)
#define PAMCMD_CLOSE_CHANNEL     (CMD_NONSTD+1)
#define PAMCMD_RESET_CHANNEL     (CMD_NONSTD+2)
#define PAMCMD_READ              (CMD_NONSTD+3)
#define PAMCMD_WRITE             (CMD_NONSTD+4)
#define PAMCMD_SEEK              (CMD_NONSTD+5)
#define PAMCMD_TELL              (CMD_NONSTD+6)
#define PAMCMD_DEVINFO           (CMD_NONSTD+7)

/* io_Error */
#define IOERR_PAMELA            -10

#endif
