#include <proto/exec.h>
#include <dos/dos.h>

#include <stdio.h>

#include "autoconf.h"
#include "debug.h"
#include "pario.h"

int main(int argc, char **argv)
{
    struct pario_handle *ph;

    puts("test-pario");
    D(("pario_init\n"));
    ph = pario_init((struct Library *)SysBase);
    if(ph != NULL) {
        D(("pario ok!\n"));

        /* show port */
        struct pario_port *port = pario_get_port(ph);
        printf("data port=%08lx  ddr=%08lx\n",
            (ULONG)port->data_port, (ULONG)port->data_ddr);
        printf("ctrl port=%08lx  ddr=%08lx\n",
            (ULONG)port->ctrl_port, (ULONG)port->ctrl_ddr);
        printf("bits: busy=%02x pout=%02x sel=%02x\n",
            port->busy_bit, port->pout_bit, port->sel_bit);
        printf("mask: busy=%02x pout=%02x sel=%02x\n",
            port->busy_mask, port->pout_mask, port->sel_mask);

        /* setup ack irq */
        BYTE ackSig = AllocSignal(-1);
        if(ackSig != -1) {
            puts("got signal");
            struct Task *myTask = FindTask(NULL);
            int error = pario_setup_ack_irq(ph, myTask, ackSig);
            if(error == 0) {
                puts("setup irq");

                puts("waiting for ack...");
                ULONG myAckMask = 1 << ackSig;
                printf("my mask %08lx  task %08lx\n", myAckMask, (ULONG)myTask);
                ULONG sigs = Wait(myAckMask | SIGBREAKF_CTRL_C);
                if(sigs & myAckMask) {
                    puts("GOT ACK!");
                } else {
                    puts("no ack!");
                }

                puts("cleanup irq");
                pario_cleanup_ack_irq(ph);
            } else {
                puts("failed irq setup");
            }
            puts("free signal");
            FreeSignal(ackSig);
        }
        puts("pario exit");
        pario_exit(ph);
        puts("done");
    } else {
        puts("error setting up pario!");
    }
    return 0;
}
