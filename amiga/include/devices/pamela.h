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
    UWORD               iopam_WireError; /* remote error */
    UWORD               iopam_Port; /* port of service to connect */
    APTR                iopam_Internal; /* internal do not use and alter */
};

/* OpenDevice() Flags */
#define PAMOB_BOOTLOADER        0         /* enter bootloader */
#define PAMOF_BOOTLOADER        (1<<0)

/* Name of Device */
#define PAMELA_NAME             "pamela.device"

/* Custom Commands */
#define PAMCMD_OPEN_CHANNEL      (CMD_NONSTD+0) // 9
#define PAMCMD_CLOSE_CHANNEL     (CMD_NONSTD+1) // 10
#define PAMCMD_RESET_CHANNEL     (CMD_NONSTD+2) // 11
#define PAMCMD_READ              (CMD_NONSTD+3) // 12
#define PAMCMD_WRITE             (CMD_NONSTD+4) // 13
#define PAMCMD_SEEK              (CMD_NONSTD+5) // 14
#define PAMCMD_TELL              (CMD_NONSTD+6) // 15
#define PAMCMD_DEVINFO           (CMD_NONSTD+7) // 16
#define PAMCMD_GET_MTU           (CMD_NONSTD+8) // 17
#define PAMCMD_SET_MTU           (CMD_NONSTD+9) // 18

/* io_Error */
#define IOERR_PAMELA            -10

#endif
