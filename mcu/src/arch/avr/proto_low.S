#include "autoconf.h"
#include <avr/io.h>
#include "pario_pins.S"

    ; some convenience macros for the control signals
    ; CLK = POUT
    ; RAK = BUSY

    .macro sbic_clk
    sbic    _SFR_IO_ADDR(PAR_POUT_PIN), PAR_POUT_BIT
    .endm

    .macro sbis_clk
    sbis    _SFR_IO_ADDR(PAR_POUT_PIN), PAR_POUT_BIT
    .endm

    .macro wait_clk_lo
l\@: sbic    _SFR_IO_ADDR(PAR_POUT_PIN), PAR_POUT_BIT
    rjmp     l\@
    .endm

    .macro wait_clk_hi
l\@: sbis    _SFR_IO_ADDR(PAR_POUT_PIN), PAR_POUT_BIT
    rjmp     l\@
    .endm

    .macro ack_hi
    sbi     _SFR_IO_ADDR(PAR_ACK_PORT), PAR_ACK_BIT
    .endm

    .macro ack_lo
    cbi     _SFR_IO_ADDR(PAR_ACK_PORT), PAR_ACK_BIT
    .endm

    .macro rak_hi
    sbi     _SFR_IO_ADDR(PAR_BUSY_PORT), PAR_BUSY_BIT
    .endm

    .macro rak_lo
    cbi     _SFR_IO_ADDR(PAR_BUSY_PORT), PAR_BUSY_BIT
    .endm

    .macro pend_hi
    sbi     _SFR_IO_ADDR(PAR_SELECT_PORT), PAR_SELECT_BIT
    .endm

    .macro pend_lo
    cbi     _SFR_IO_ADDR(PAR_SELECT_PORT), PAR_SELECT_BIT
    .endm

    ; if needed use select pin as a debug line
#ifdef CONFIG_DEBUG_PIN
    /* lo is active, hi is inactive */
    .macro debug_lo
    sbi     _SFR_IO_ADDR(PAR_SELECT_PORT), PAR_SELECT_BIT
    .endm
    .macro debug_hi
    cbi     _SFR_IO_ADDR(PAR_SELECT_PORT), PAR_SELECT_BIT
    .endm
#else
    .macro debug_lo
    .endm
    .macro debug_hi
    .endm
#endif

    .section .text

    ; ----- proto_low_init -----
    ; void proto_low_init(void)
    ; setup parallel port lines
    .global  proto_low_init
proto_low_init:
    ; setup data port: idle mode
    ddr_idle
    ; setup control lines
    ; DDR INPUT: strobe, pout
    cbi     _SFR_IO_ADDR(PAR_STROBE_DDR), PAR_STROBE_BIT
    cbi     _SFR_IO_ADDR(PAR_POUT_DDR), PAR_POUT_BIT
    ; DDR OUTPUT: ack, busy, select
    sbi     _SFR_IO_ADDR(PAR_SELECT_DDR), PAR_SELECT_BIT
    sbi     _SFR_IO_ADDR(PAR_ACK_DDR), PAR_ACK_BIT
    sbi     _SFR_IO_ADDR(PAR_BUSY_DDR), PAR_BUSY_BIT
    ; set OUTPUT levels
    ack_hi
    rak_hi
    pend_hi
    ret


    ; ----- ack/pend signals -----
    ; void proto_low_ack_lo(void)
    .global  proto_low_ack_lo
proto_low_ack_lo:
    ack_lo
    ret

    ; void proto_low_ack_hi(void)
    .global  proto_low_ack_hi
proto_low_ack_hi:
    ack_hi
    ret

    ; void proto_low_pend_lo(void)
    .global  proto_low_pend_lo
proto_low_pend_lo:
    pend_lo
    ret

    ; void proto_low_pend_hi(void)
    .global  proto_low_pend_hi
proto_low_pend_hi:
    pend_hi
    ret


    ; ----- proto_low_get_cmd -----
    ; u08 proto_low_get_cmd(void)
    ; return 0 or command byte
    .global  proto_low_get_cmd
proto_low_get_cmd:
    clr     r25
    clr     r24
    sbic_clk
    ret
    data_in
    mov     r24, r18
    ret


    ; ----- proto_low_action -----
    ; void proto_low_action(void)
    ; confirm command with no value
    ; CL | WL | CH
    .global  proto_low_action
proto_low_action:
    ; when we enter here the clk from master is already low
    ; now signal rak lo to the master. we are now in sync
    rak_lo

    ; command finishes with a last clk hi transition
    wait_clk_hi
    ; confirm it
    rak_hi
    ret


    ; ----- proto_low_write_word -----
    ; master wants to write a u16 register value
    ; u08 proto_low_write_word(u16 *val)
    ; in:  r24,r25 = wrote val ptr
    ; out: r24 = reg_num
    .global  proto_low_write_word
proto_low_write_word:
    ; Z is buffer pointer
    movw    r30,r24

    ; confirm to master we are ready
    rak_lo

    ; ddr: input
    wait_clk_hi
    ddr_in

    ; read reg_num
    wait_clk_lo
    data_in
    mov     r24, r18  ; reg_num is return value

    ; byte 1
    ; wait clk
    wait_clk_hi
    ; get data and save as result
    data_in
    st      Z+,r18

    ; byte 2
    ; wait clk
    wait_clk_lo
    data_in
    st      Z+,r18

    ; unused cycle
    wait_clk_hi

    ; ddr: idle
    wait_clk_lo
    ddr_idle

    ; end sync
    wait_clk_hi
    rak_hi
    ret


    ; ----- proto_low_read_word -----
    ; master wants to read a u16 register value
    ; u08 proto_low_read_word(u16 v)
    ; in:  r24,r25 = read val
    ; out: r24 = reg_num
    .global  proto_low_read_word
proto_low_read_word:
    rak_lo

    ; ddr: input
    wait_clk_hi
    ddr_in

    ; read reg_num
    wait_clk_lo
    data_in
    mov     r23, r18  ; temp=r23

    ; ddr: output
    wait_clk_hi
    ddr_out

    ; byte 1
    mov     r18, r25
    data_out_prepare
    wait_clk_lo
    data_out_set

    ; byte 2
    mov     r18, r24
    data_out_prepare
    wait_clk_hi
    data_out_set

    ; ddr: idle
    wait_clk_lo
    ddr_idle

    ; end sync
    wait_clk_hi
    rak_hi

    ; return value
    mov     r24, r23
    ret


    ; ----- proto_low_write_block -----
    ; master wants to write a message
    ; u16 proto_low_write_block(u16 max_words, u08 *buffer, u16 *chn_ext)
    ; in:  r25,r24 = max_words we can receive
    ; in:  r23,r22 = buffer ptr
    ; in:  r21,r20 = ptr to (extra << 8 | channel)
    ; out: r25,r24 = num_words received
    .global  proto_low_write_block
proto_low_write_block:
    ; Z is buffer pointer
    movw    r30,r22
    ; X is ptr to extra/channel
    movw    r26,r20

    ; confirm to master we are ready
    rak_lo

    ; ddr input
    wait_clk_hi
    ddr_in
    debug_hi

    ; -- read extra, channel
    ; extra
    wait_clk_lo
    data_in
    mov     r0,r18
    debug_lo

    ; channel
    wait_clk_hi
    data_in
    st      x+,r18
    st      x+,r0
    debug_hi

    ; -- read size: hi/lo=r23:r22
    ; hi
    wait_clk_lo
    data_in
    mov     r23,r18
    debug_lo
    ; lo
    wait_clk_hi
    data_in
    mov     r22,r18
    debug_hi

    ; empty size?
    or      r18,r23
    breq    .wb_empty

    ; check size: too large?
    clt
    cp      r24,r22
    cpc     r25,r23
    brsh    .wb_size_ok

    ; size invalid - set T flag
    set

.wb_size_ok:
    ; save result size
    movw    r24,r22

    ; double size: convert to bytes
    add     r22,r22
    adc     r23,r23

    ; add start address: end address: r23:r22
    add     r22,r30
    adc     r23,r31

    ; if size is invalid use a fake loop
    brts    .wb_fake_loop

    ; copy loop
.wb_loop:
    ; check end
    cp      r30,r22
    cpc     r31,r23
    breq    .wb_end

    ; odd byte
    wait_clk_lo
    data_in
    st      Z+,r18
    debug_lo
    ; even byte
    wait_clk_hi
    data_in
    st      Z+,r18
    debug_hi

    rjmp    .wb_loop

    ; non copy fake loop
.wb_fake_loop:
    ; check end
    cp      r30,r22
    cpc     r31,r23
    breq    .wb_end

    ; odd byte
    wait_clk_lo
    adiw    r30,1
    debug_lo
    ; even byte
    wait_clk_hi
    adiw    r30,1
    debug_hi

    rjmp    .wb_fake_loop

.wb_empty:
    ; empty message
    clr     r25
    clr     r24
.wb_end:
    ; ddr idle
    wait_clk_lo
    ddr_idle
    debug_lo

    ; end sync
    wait_clk_hi
    rak_hi
    ret


    ; ----- proto_low_read_block -----
    ; master wants to read a message
    ; void proto_low_read_block(u16 num_words, const u08 *buffer, u16 chn_ext)
    ; in:  r25,r24 = max_words we can receive
    ; in:  r23,r22 = buffer ptr
    ; in:  r21,r20 = extra << 8 | channel value
    ; out: -
    .global  proto_low_read_block
proto_low_read_block:
    ; Z is buffer pointer
    movw    r30,r22

    ; save r20 (channel) -> r0
    ; warning: r20 gets globbered by first data_out* call!
    mov     r0,r20

    ; confirm to master we are ready
    rak_lo

    ; ddr out
    wait_clk_hi
    ddr_out
    debug_hi

    ; send extra
    mov     r18,r21
    data_out_prepare
    wait_clk_lo
    data_out_set
    debug_lo

    ; send channel
    mov     r18,r0
    data_out_prepare
    wait_clk_hi
    data_out_set
    debug_hi

    ; size: hi
    mov     r18,r25
    data_out_prepare
    wait_clk_lo
    data_out_set
    debug_lo

    ; size: lo
    mov     r18, r24
    data_out_prepare
    wait_clk_hi
    data_out_set
    debug_hi

    ; convert size to bytes
    add     r24,r24
    adc     r25,r25

    ; set end address: r24:r25
    add     r24,r30
    adc     r25,r31

    ; copy loop
.rb_loop:
    ; check end
    cp      r30,r24
    cpc     r31,r25
    breq    .rb_end

    ; odd byte
    ld      r18,Z+
    data_out_prepare
    wait_clk_lo
    data_out_set
    debug_lo

    ; even byte
    ld      r18,Z+
    data_out_prepare
    wait_clk_hi
    data_out_set
    debug_hi

    rjmp    .rb_loop
.rb_end:
    ; ddr idle
    wait_clk_lo
    ddr_idle
    debug_lo

    ; end sync
    wait_clk_hi
    rak_hi
    ret

    .end
