        include "exec/macros.i"

        xdef _pario_irq_handler

        ; a1 = pario_handle
_pario_irq_handler:
        move.l  (a1),a6             ; sysbase
        move.l  8(a1),d0            ; sigMask
        move.l  4(a1),a1            ; sigTask
        JSRLIB  Signal
        moveq #0,d0
        rts
