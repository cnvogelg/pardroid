#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include "autoconf.h"
#include "debug.h"
#include "pario.h"
#include "pario_port.h"

int dosmain(void)
{
    struct pario_handle *ph;

    PutStr("test-pario\n");
    D(("pario_init\n"));
    ph = pario_init((struct Library *)SysBase);
    if(ph != NULL) {
        D(("pario ok!\n"));

        /* show port */
        struct pario_port *port = pario_get_port(ph);
        Printf("data port=%08lx  ddr=%08lx\n",
            (ULONG)port->data_port, (ULONG)port->data_ddr);
        Printf("ctrl port=%08lx  ddr=%08lx\n",
            (ULONG)port->ctrl_port, (ULONG)port->ctrl_ddr);
        Printf("bits: busy=%02lx pout=%02lx sel=%02lx\n",
            (ULONG)port->busy_bit, (ULONG)port->pout_bit, (ULONG)port->sel_bit);
        Printf("mask: busy=%02lx pout=%02lx sel=%02lx\n",
            (ULONG)port->busy_mask, (ULONG)port->pout_mask, (ULONG)port->sel_mask);

        /* write some bytes */
        PutStr("writing bytes\n");
        *port->data_ddr = 0xff;
        for(int i=0;i<256;i++) {
            *port->data_port=i;
        }
        *port->data_ddr = 0x00;
        PutStr("done\n");

        /* write signals */
        PutStr("toggle control\n");
        *port->ctrl_ddr |= port->all_mask;
        for(int i=0;i<100;i++) {
            *port->ctrl_port |= port->busy_mask;
            *port->ctrl_port &= ~port->busy_mask;

            *port->ctrl_port |= port->pout_mask;
            *port->ctrl_port &= ~port->pout_mask;

            *port->ctrl_port |= port->sel_mask;
            *port->ctrl_port &= ~port->sel_mask;
        }
        *port->ctrl_ddr &= ~port->all_mask;
        PutStr("done\n");

        /* setup ack irq */
        BYTE ackSig = AllocSignal(-1);
        if(ackSig != -1) {
            PutStr("got signal\n");
            struct Task *myTask = FindTask(NULL);
            int error = pario_setup_ack_irq(ph, myTask, ackSig);
            if(error == 0) {
                PutStr("setup irq\n");

                PutStr("waiting for ack...\n");
                ULONG myAckMask = 1 << ackSig;
                Printf("my mask %08lx  task %08lx\n", myAckMask, (ULONG)myTask);
                ULONG sigs = Wait(myAckMask | SIGBREAKF_CTRL_C);
                if(sigs & myAckMask) {
                    PutStr("GOT ACK!\n");
                } else {
                    PutStr("no ack!\n");
                }

                PutStr("cleanup irq\n");
                pario_cleanup_ack_irq(ph);
            } else {
                PutStr("failed irq setup\n");
            }
            PutStr("free signal\n");
            FreeSignal(ackSig);
        }
        PutStr("pario exit\n");
        pario_exit(ph);
        PutStr("done\n");
    } else {
        PutStr("error setting up pario!\n");
    }
    return 0;
}
