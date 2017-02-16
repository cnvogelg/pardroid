; --- proto signals ---
; POUT = /CLK  (Amiga out)
; BUSY = /RAK  (Amiga in)
; SEL  = ?

        include         "pario.i"
        include         "proto.i"

        xdef            _proto_low_ping
        xdef            _proto_low_test_write
        xdef            _proto_low_test_read

; ----- macros --------------------------------------------------------------

; --- setup_port_regs ---
; in:  a0 = port_ptr
; out: d2 = rak bit
;      d3 = clk bit
;      d4 = old ctrl val
;      a3 = data port
;      a4 = data ddr
;      a5 = ctrl port
setup_port_regs  MACRO
        moveq   #0,d2
        moveq   #0,d3
        move.b  PO_BUSY_BIT(a0),d2
        move.b  PO_POUT_BIT(a0),d3
        move.l  PO_DATA_PORT(a0),a3
        move.l  PO_DATA_DDR(a0),a4
        move.l  PO_CTRL_PORT(a0),a5
        ; read current ctrl val
        move.b  (a5),d4
        ENDM


; --- check_rak_hi ---
; check if RAK is high otherwise rts with error
; \1 end label
check_rak_hi  MACRO
        btst    d2,(a5)
        bne.s   \@
        moveq   #RET_RAK_INVALID,d0
        bra     \1
\@:
        ENDM


; --- check_rak_lo ---
; check if RAK is high otherwise rts with error
; \1 end label
check_rak_lo  MACRO
        btst    d2,(a5)
        beq.s   \@
        moveq   #RET_SLAVE_ERROR,d0
        bra     \1
\@:
        ENDM


; --- clk_lo ---
; set CLK signal to low
clk_lo  MACRO
        bclr    d3,d4
        move.b  d4,(a5)
        ENDM


; --- clk_hi ---
; set CLK signal to high
clk_hi  MACRO
        bset    d3,d4
        move.b  d4,(a5)
        ENDM


; --- wait_rak_lo ---
; wait for RAK to become low or if timeout triggers
; \1 = jump label on timeout
wait_rak_lo  MACRO
        ; check RAK level
\@1:    btst    d2,(a5)
        beq.s   \@2
        ; check for timeout
        tst.b   (a1)
        beq.s   \@1
        ; error
        moveq   #RET_TIMEOUT,d0
        bra     \1
\@2:
        ENDM


; --- wait_rak_hi ---
; wait for RAK to become high or if timeout triggers
; \1 = jump label on timeout
wait_rak_hi  MACRO
        ; check RAK level
\@1:    btst    d2,(a5)
        bne.s   \@2
        ; check for timeout
        tst.b   (a1)
        beq.s   \@1
        ; error
        moveq   #RET_TIMEOUT,d0
        bra     \1
\@2:
        ENDM


; --- ddr_in ---
; set data direction to input
ddr_in  MACRO
        sf.b    (a4)
        ENDM


; --- ddr_out ---
; set data direction to output
ddr_out  MACRO
        st.b    (a4)
        ENDM


; --- set_cmd_idle ---
; set command to idle (0)
set_cmd_idle  MACRO
        sf.b    (a3)
        ENDM


; --- set_cmd ---
; set a command byte to data port
; \1 = cmd constant
set_cmd  MACRO
        move.b  \1,(a3)
        ENDM


; --- set_data ---
; set data port
; \1 = value to set
set_data MACRO
        move.b  \1,(a3)
        ENDM


; --- get_data ---
; get data from port
; \1 = store value
get_data MACRO
        move.b  (a3),\1
        ENDM


; ----- functions -----------------------------------------------------------

; --- proto_low_ping ---
; handle ping command
;
;   in:
;       a0      struct pario_port *port
;       a1      volatile UBYTE *timeout_flag
;       d0      CMD_PING constant
;   out:
;       d0      return code
_proto_low_ping:
        movem.l d2-d7/a2-a6,-(sp)

        ; setup regs with port values and read old ctrl value
        setup_port_regs

        ; -- sync with slave
        ; check RAK to be high or abort
        check_rak_hi    plp_end
        ; set cmd to data port
        set_cmd         d0
        ; set CLK to low (active) to trigger command at slave
        clk_lo
        ; busy wait with timeout for RAK to go low
        ; (we wait for the slave to react/sync)
        wait_rak_lo     plp_abort

        ; now we are in sync with slave
        ; we are now in work phase
        ; but ping does nothing here

        ; -- final sync
        ; now raise CLK again
        clk_hi
        ; slave returns hi if command was ok otherwise
        wait_rak_hi  plp_end

        ; ok
        moveq   #RET_OK,d0
plp_end:
        ; restore cmd
        set_cmd_idle

        movem.l (sp)+,d2-d7/a2-a6
        rts
plp_abort:
        ; ensure CLK is hi
        clk_hi
        bra.s   plp_end


; --- proto_low_test_write ---
; in:  a0 = port ptr
;      a1 = timeout byte ptr
;      a2 = ptr to byte
; out: d0 = result
_proto_low_test_write:
        movem.l d2-d7/a2-a6,-(sp)
        setup_port_regs

        ; sync with slave
        check_rak_hi    pltw_end
        set_cmd         #CMD_TEST_WRITE
        clk_lo
        wait_rak_lo     pltw_abort

        ; -- first byte
        ; setup test value on data lines
        set_data        (a2)+
        ; signal to slave to read the value
        clk_hi

        ; -- second byte
        set_data        (a2)+
        clk_lo

        ; done, read slave state
        ; if slave already pulled high then an error was found
        check_rak_lo    pltw_abort

        ; final sync
        clk_hi
        ; wait for slave done
        wait_rak_hi     pltw_end

        ; ok
        moveq   #RET_OK,d0
pltw_end:
        set_cmd_idle
        movem.l (sp)+,d2-d7/a2-a6
        rts
pltw_abort:
        ; ensure CLK is hi
        clk_hi
        bra.s    pltw_end


; --- proto_low_test_read ---
; in:  a0 = port ptr
;      a1 = timeout byte ptr
;      a2 = ptr to test byte
; out: d0 = result
_proto_low_test_read:
        movem.l d2-d7/a2-a6,-(sp)
        setup_port_regs

        ; sync with slave
        check_rak_hi    pltr_end
        set_cmd         #CMD_TEST_READ
        clk_lo
        wait_rak_lo     pltr_abort

        ; switch: port read
        ddr_in
        clk_hi

        ; first byte
        ; signal read to slave
        clk_lo
        ; read value from data port
        get_data        (a2)+

        ; second bytes
        clk_hi
        get_data        (a2)+

        ; switch: port write
        clk_lo
        ddr_out

        ; read status
        check_rak_lo    pltr_abort

        ; final sync
        clk_hi
        wait_rak_hi     pltr_end

        ; ok
        moveq   #RET_OK,d0
pltr_end:
        set_cmd_idle
        movem.l (sp)+,d2-d7/a2-a6
        rts
pltr_abort:
        ; ensure CLK is hi
        clk_hi
        bra.s    pltr_end
