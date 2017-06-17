; --- proto signals ---
; POUT = /CLK  (Amiga out)
; BUSY = /RAK  (Amiga in)
; SEL  = /PEND (Amiga in)

        include         "pario.i"
        include         "proto.i"

        xdef            _proto_low_get_status
        xdef            _proto_low_action
        xdef            _proto_low_action_bench
        xdef            _proto_low_write_word
        xdef            _proto_low_read_word
        xdef            _proto_low_write_block
        xdef            _proto_low_read_block

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


; --- wait_rak_lo_pend_abort ---
; wait for RAK to become low or if timeout triggers or abort if pending is set
; \1 = jump label on timeout
; \2 = register with pending bit
wait_rak_lo_pend_abort  MACRO
        ; check RAK level
\@1:    btst    d2,(a5)
        beq.s   \@2
        ; check for timeout
        tst.b   (a1)
        beq.s   \@3
        ; timeout error
        moveq   #RET_TIMEOUT,d0
        bra     \1
\@3:    ; check pending (low active)
        btst    \2,(a5)
        bne.s   \@1
        ; abort write due to pending read
        moveq   #RET_WRITE_ABORT,d0
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


; --- ddr_idle ---
; \1 = temp reg
; set special idle mode ddr: bit 0..3 out, bit 4..7 in
ddr_idle  MACRO
        moveq   #15,\1
        move.b  \1,(a4)
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


; --- call_callback ---
; \1 = id
call_cb MACRO
        moveq   \1,d0
        movem.l a0-a1,-(sp)
        jsr     (a0)
        movem.l (sp)+,a0-a1
        ENDM


; --- cflag_lo ---
; \1 = reg for sel bit
cflg_lo MACRO
        move.b          PO_SEL_BIT(a0),\1
        bclr            \1,d4
        move.b          d4,(a5)
        ENDM


; --- cflag_hi ---
; \1 = reg for sel bit
cflg_hi MACRO
        move.b          PO_SEL_BIT(a0),\1
        bset            \1,d4
        move.b          d4,(a5)
        ENDM


; ----- functions -----------------------------------------------------------

; --- proto_low_get_status ---
; read the status nybble of the device from the idle byte
;
;   in:
;       a0      struct pario_port *port
;   out:
;       d0      return status bits in high nybble
_proto_low_get_status:
        move.l          PO_DATA_PORT(a0),a1
        move.l          PO_CTRL_PORT(a0),a0
        move.b          PO_SEL_BIT(a0),d1

        ; clfg_lo to signal status access
        bclr            d1,(a0)

        ; read status
        move.b          (a1),d0
        andi.b          #$f0,d0

        ; cflg_hi to signal end of status access
        bset            d1,(a0)

        rts


; --- proto_low_action ---
; a simple command that does not transfer any value
;
;   in:
;       a0      struct pario_port *port
;       a1      volatile UBYTE *timeout_flag
;       d0      action constant
;   out:
;       d0      return code
_proto_low_action:
        movem.l d2-d7/a2-a6,-(sp)

        ; setup regs with port values and read old ctrl value
        setup_port_regs

        ; -- sync with slave
        ; check RAK to be high or abort
        check_rak_hi    pla_end
        ; set cmd to data port
        set_cmd         d0
        ; set cflag to signal command
        cflg_lo         d1
        ; set CLK to low (active) to trigger command at slave
        clk_lo
        ; busy wait with timeout for RAK to go low
        ; (we wait for the slave to react/sync)
        wait_rak_lo     pla_abort

        ; now we are in sync with slave
        ; we are now in work phase
        ; but ping does nothing here

        ; -- final sync
        ; now raise CLK again
        clk_hi
        ; expect slave to raise rak, too
        check_rak_hi    pla_end

        ; ok
        moveq   #RET_OK,d0
pla_end:
        ; restore cmd
        set_cmd_idle
        ; restore cflg
        cflg_hi         d1

        movem.l (sp)+,d2-d7/a2-a6
        rts
pla_abort:
        ; ensure CLK is hi
        clk_hi
        bra.s   pla_end


; --- proto_low_action_bench ---
; the proto_low_action routine instrumented with benchmark callbacks
;
;   in:
;       a0      struct pario_port *port
;       a1      volatile UBYTE *timeout_flag
;       a2      callback struct
;       d0      CMD_PING constant
;   out:
;       d0      return code
_proto_low_action_bench:
        movem.l d2-d7/a2-a6,-(sp)

        ; setup regs with port values and read old ctrl value
        setup_port_regs

        ; retrieve pointer for callback
        move.l          (a2),a0

        ; -- sync with slave
        ; check RAK to be high or abort
        check_rak_hi    plab_end
        ; set cmd to data port
        set_cmd         d0
        ; set cflag to signal command
        cflg_lo         d1
        ; set CLK to low (active) to trigger command at slave
        clk_lo

        ; callback 0: set clock low
        call_cb         #0

        ; busy wait with timeout for RAK to go low
        ; (we wait for the slave to react/sync)
        wait_rak_lo     plab_abort

        ; callback 1: got rak lo
        call_cb         #1

        ; -- final sync
        ; now raise CLK again
        clk_hi
        ; expect slave to raise rak, too
        check_rak_hi    plab_end

        ; callback 2: got rak hi
        call_cb         #2

        ; ok
        moveq   #RET_OK,d0
plab_end:
        ; restore cmd
        set_cmd_idle
        ; restore cflg
        cflg_hi         d1

        movem.l (sp)+,d2-d7/a2-a6
        rts
plab_abort:
        ; ensure CLK is hi
        clk_hi
        bra.s   plab_end


; --- proto_low_write_word ---
; in:  a0 = port ptr
;      a1 = timeout byte ptr
;      a2 = ptr to data
;      d0 = cmd byte
; out: d0 = result
_proto_low_write_word:
        movem.l d2-d7/a2-a6,-(sp)
        setup_port_regs

        ; sync with slave
        check_rak_hi    plrw_end
        set_cmd         d0
        cflg_lo         d1
        clk_lo
        wait_rak_lo     plrw_abort

        ; ddr: out
        clk_hi
        ddr_out

        ; -- first byte
        ; setup test value on data lines
        set_data        (a2)+
        ; signal to slave to read the value
        clk_lo

        ; -- second byte
        set_data        (a2)+
        clk_hi

        ; ddr: idle
        clk_lo
        ddr_idle        d7

        ; final sync
        clk_hi
        ; wait for slave done
        check_rak_hi    plrw_end

        ; ok
        moveq   #RET_OK,d0
plrw_end:
        set_cmd_idle
        cflg_hi         d1
        movem.l (sp)+,d2-d7/a2-a6
        rts
plrw_abort:
        ; ensure CLK is hi
        clk_hi
        bra.s    plrw_end


; --- proto_low_read_word ---
; in:  a0 = port ptr
;      a1 = timeout byte ptr
;      a2 = ptr to test byte
;      d0 = cmd byte
; out: d0 = result
_proto_low_read_word:
        movem.l d2-d7/a2-a6,-(sp)
        setup_port_regs

        ; sync with slave
        check_rak_hi    plrr_end
        set_cmd         d0
        cflg_lo         d1
        clk_lo
        wait_rak_lo     plrr_abort

        ; ddr: in
        ddr_in
        clk_hi

        ; first byte
        ; signal read to slave
        clk_lo
        ; read value from data port
        get_data        (a2)+

        ; second byte
        clk_hi
        get_data        (a2)+

        ; ddr: idle
        clk_lo
        ddr_idle        d7

        ; final sync
        clk_hi
        check_rak_hi    plrr_end

        ; ok
        moveq   #RET_OK,d0
plrr_end:
        set_cmd_idle
        cflg_hi         d1
        movem.l (sp)+,d2-d7/a2-a6
        rts
plrr_abort:
        ; ensure CLK is hi
        clk_hi
        bra.s    plrr_end


; --- proto_low_write_block ---
; in:  a0 = port ptr
;      a1 = timeout byte ptr
;      a2 = ptr to blkiov
;      d0 = cmd
; out: d0 = result
;
; blkiov: ULONG[]
; [0]  total size of block in words | (extra << 24) | (channel << 16)
; [1]  first chunk size in words
; [2]  pointer to data of first chunk
; [3]  second chunk size in words
; [4]  pointer to second chunk data
; ...
; [n]  0 (size) (last chunk tag)
_proto_low_write_block:
        movem.l d2-d7/a2-a6,-(sp)
        setup_port_regs
        ; addtionally prepare pending bit
        moveq           #0,d7
        move.b          PO_SEL_BIT(a0),d7

        ; sync with slave
        check_rak_hi    plrw_end
        set_cmd         d0
        clk_lo

        ; prepare size while waiting for slave
        move.l          (a2)+,d1 ; total size in words
        move.w          d1,d5
        lsr.w           #8,d5

        ; prepare extra/channel values (high word of iov[0])
        move.l          d1,d6
        swap            d6
        move.w          d6,d0 ; d6 = channel
        lsr.w           #8,d0 ; d0 = extra

        ; wait for slave sync (abort if pending was set, or on timeout)
        wait_rak_lo_pend_abort     plrw_abort,d7

        ; switch port to write
        clk_hi
        ddr_out

        ; send extra/channel
        set_data        d0 ; extra
        clk_lo
        set_data        d6 ; channel
        clk_hi

        ; send size
        set_data        d5 ; hi byte
        clk_lo
        set_data        d1 ; lo byte
        clk_hi

        ; empty message?
        tst.w           d1
        beq.s           plmw_done

plmw_chunks:
        ; get chunk size and buffer pointer
        move.l          (a2)+,d1
        ; last chunk?
        tst.w           d1
        beq.s           plmw_done
        ; enter chunk copy loop
        subq.w          #1,d1 ; for dbra
        move.l          (a2)+,a0

        ; data block loop
plmw_loop:
        ; odd byte
        set_data        (a0)+
        clk_lo
        ; even byte
        set_data        (a0)+
        clk_hi

        dbra            d1,plmw_loop
        bra.s           plmw_chunks

plmw_done:
        ; set ddr to idle
        clk_lo
        ddr_idle        d7

        ; final sync
        clk_hi
        check_rak_hi    plmw_end

        ; ok
        moveq   #RET_OK,d0
plmw_end:
        set_cmd_idle
        movem.l (sp)+,d2-d7/a2-a6
        rts
plmw_abort:
        ; ensure CLK is hi
        clk_hi
        bra.s    plmw_end


; --- proto_low_read_block ---
; in:  a0 = port ptr
;      a1 = timeout byte ptr
;      a2 = ptr to blkiov (see above)
;      d0 = cmd byte
; out: d0 = result
;      blkiov[0] = extra << 24 | channel << 16 | recv_words
_proto_low_read_block:
        movem.l d2-d7/a2-a6,-(sp)
        setup_port_regs

        ; sync with slave
        check_rak_hi    plmr_end
        set_cmd         d0
        clk_lo

        ; prepare size regs
        moveq           #0,d5
        moveq           #0,d6
        moveq           #0,d7
        ; get max size given in blkiov
        move.l          (a2),d1

        ; final slave sync
        wait_rak_lo     plmr_abort

        ; switch: port read
        ddr_in
        clk_hi

        ; read channel/extra
        clk_lo
        get_data        d7 ; extra
        clk_hi
        get_data        d6 ; channel

        ; combine (extra+channel) and move to high word of d7
        lsl.w           #8,d7
        or.w            d6,d7
        swap            d7

        ; read size
        clk_lo
        get_data        d5 ; hi size
        clk_hi
        get_data        d6 ; lo size

        ; combine size lo/hi -> d5
        lsl.w           #8,d5
        or.w            d6,d5
        ; store real size in blkiov[0] (combined with channel+extra)
        or.w            d5,d7
        move.l          d7,(a2)+
        ; check size
        tst.w           d5 ; empty message
        beq.s           plmr_done_ok
        cmp.w           d1,d5 ; max size
        bls.s           plmr_chunk

        ; size invalid - run a fake loop a do not store data
        moveq           #RET_MSG_TOO_LARGE,d0
        subq.w          #1,d5
plmr_fake_loop:
        clk_lo
        get_data        d1
        clk_hi
        get_data        d1
        dbra            d5,plmr_fake_loop
        bra.s           plmr_done

plmr_chunk:
        ; get current chunk size and pointer
        move.l          (a2)+,d0
        move.l          (a2)+,a0
        ; is remaining read size less than chunk?
        cmp.w           d5,d0
        bls.s           plmr_keep
        ; don't fill whole chunk but only what is left
        move.w          d5,d0
plmr_keep:
        ; sub current loop size from total size
        sub.w           d0,d5
        ; run a copy loop and store data
        subq.w          #1,d0
        ; data copy loop
plmr_copy_loop:
        ; odd byte
        clk_lo
        get_data        (a0)+
        ; even byte
        clk_hi
        get_data        (a0)+
        dbra            d0,plmr_copy_loop

        ; done?
        tst.w           d5
        beq.s           plmr_done_ok
        ; nope. next chunk
        bra.s           plmr_chunk

plmr_done_ok:
        ; ok
        moveq   #RET_OK,d0

plmr_done:
        ; switch: port write
        clk_lo
        ddr_idle        d7

        ; final sync
        clk_hi
        check_rak_hi    plmr_end
plmr_end:
        set_cmd_idle
        movem.l (sp)+,d2-d7/a2-a6
        rts
plmr_abort:
        ; ensure CLK is hi
        clk_hi
        bra.s           plmr_end

