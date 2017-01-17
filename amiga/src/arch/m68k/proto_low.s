; --- proto signals ---
; POUT = /CLK  (Amiga out)
; BUSY = /RAK  (Amiga in)
; SEL  = /PEND (Amiga in) (?)

        include         "pario.i"

        xdef            _proto_low_ping

; error/return codes
RET_OK                  equ     0
RET_RAK_INVALID         equ     1
RET_TIMEOUT             equ     2

; --- proto_low_ping ---
; handle ping command
;
;   in:
;       a0      struct pario_port *port
;       a1      volatile UBYTE *timeout_flag
;   out:
;       d0      return code
_proto_low_ping:
        ; check RAK to be high
        move.b  PO_BUSY_BIT(a0),d0
        btst    d0,PO_CTRL_PORT(a0)
        bne.s   .1
        moveq   #RET_RAK_INVALID,d0
        rts
.1:
        ; set CLK to low (active)
        move.b  PO_POUT_BIT(a0),d0
        bclr    d0,PO_CTRL_PORT(a0)

        ; busy wait with timeout for RAK to go low
        move.b  PO_BUSY_BIT(a0),d0
.2:     btst    d0,PO_CTRL_PORT(a0)
        beq.s   .3
        tst.b   (a1)
        beq.s   .2
        moveq   #RET_TIMEOUT,d0
        rts
.3:
        ; now raise CLK again
        move.b  PO_POUT_BIT(a0),d0
        bset    d0,PO_CTRL_PORT(a0)

        ; wait for RAK to return to high again
        move.b  PO_BUSY_BIT(a0),d0
.4:     btst    d0,PO_CTRL_PORT(a0)
        bne.s   .5
        tst.b   (a1)
        beq.s   .4
        moveq   #RET_TIMEOUT,d0
        rts
.5:
        moveq   #RET_OK,d0
        rts

