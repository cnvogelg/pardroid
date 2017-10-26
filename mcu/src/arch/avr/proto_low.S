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

    .macro sbic_cflg
    sbic    _SFR_IO_ADDR(PAR_SELECT_PIN), PAR_SELECT_BIT
    .endm

    .macro sbis_cflg
    sbis    _SFR_IO_ADDR(PAR_SELECT_PIN), PAR_SELECT_BIT
    .endm

    .macro wait_clk_lo
l\@: sbic    _SFR_IO_ADDR(PAR_POUT_PIN), PAR_POUT_BIT
    rjmp     l\@
    .endm

    .macro wait_clk_lo_abort_cfg_lo exit
l\@:
    sbis_cflg
    rjmp     \exit
    sbic    _SFR_IO_ADDR(PAR_POUT_PIN), PAR_POUT_BIT
    rjmp     l\@
    .endm

    .macro wait_clk_hi
l\@: sbis    _SFR_IO_ADDR(PAR_POUT_PIN), PAR_POUT_BIT
    rjmp     l\@
    .endm

    .macro wait_cflg_lo
l\@: sbic    _SFR_IO_ADDR(PAR_SELECT_PIN), PAR_SELECT_BIT
    rjmp     l\@
    .endm

    .macro wait_cflg_hi
l\@: sbis    _SFR_IO_ADDR(PAR_SELECT_PIN), PAR_SELECT_BIT
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

    .macro sbic_rak
    sbic    _SFR_IO_ADDR(PAR_BUSY_PIN), PAR_BUSY_BIT
    .endm

    .macro sbis_rak
    sbis    _SFR_IO_ADDR(PAR_BUSY_PIN), PAR_BUSY_BIT
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

    ; ----- proto_low_init -----
    ; void proto_low_init(u08 status)
    ; setup parallel port lines
    ; in: r24 = status bits
    .section .text.proto_low_init
    .global  proto_low_init
proto_low_init:
    ; setup data port: idle mode
    ddr_idle
    ; set status bits (4,5,6,7)
    andi    r24,0xf0
    ori     r24,0x0f
    mov     r18,r24
    data_out_prepare
    data_out_set
    ; setup control lines
    ; DDR INPUT: strobe, pout, select
    cbi     _SFR_IO_ADDR(PAR_STROBE_DDR), PAR_STROBE_BIT
    cbi     _SFR_IO_ADDR(PAR_POUT_DDR), PAR_POUT_BIT
    cbi     _SFR_IO_ADDR(PAR_SELECT_DDR), PAR_SELECT_BIT
    ; DDR OUTPUT: ack, busy
    sbi     _SFR_IO_ADDR(PAR_ACK_DDR), PAR_ACK_BIT
    sbi     _SFR_IO_ADDR(PAR_BUSY_DDR), PAR_BUSY_BIT
    ; set OUTPUT levels
    ack_hi
    rak_hi
    ret


    ; ----- device status bits -----
    ; u08 proto_low_set_status(u08 status)
    ; in: r24 = status bits
    ; out: r24 = 0=abort, 1=done
    .section .text.proto_low_set_status
    .global  proto_low_set_status
proto_low_set_status:
    ; if rak is low then abort
    ; i.e. we are inside an action/function and master is waiting
    ; we will restore the state after the action/function
    sbis_rak
    rjmp    plss_abort
    ; if cflg is high then perform change
    ; i.e. we are not inside a action/function nor in a master update
    sbic_cflg
    rjmp    plss_ok
    ; if clk is low then perform change
    ; i.e. cflg is low but we are waiting for action/function to begin
    sbis_clk
    rjmp    plss_ok
    ; wait
    rjmp    proto_low_set_status
plss_abort:
    clr     r24
    ret
plss_ok:
    ; perform change
    andi    r24,0xf0
    ori     r24,0x0f
    mov     r18,r24
    data_out_prepare
    data_out_set
    ldi     r24,1
    ret


    ; ----- wait for end of invalid action/func -----
    ; void proto_low_wait_cflg_hi(void)
    .section .text.proto_low_wait_cflg_hi
    .global  proto_low_wait_cflg_hi
proto_low_wait_cflg_hi:
    wait_cflg_hi
    ret


    ; ----- ack/pend signals -----
    ; void proto_low_ack_lo(void)
    .section .text.proto_low_ack_lo
    .global  proto_low_ack_lo
proto_low_ack_lo:
    ack_lo
    ret

    ; void proto_low_ack_hi(void)
    .section .text.proto_low_ack_hi
    .global  proto_low_ack_hi
proto_low_ack_hi:
    ack_hi
    ret

    ; ----- proto_low_get_cmd -----
    ; u08 proto_low_get_cmd(void)
    ; return 0xff for idle or command byte
    .section .text.proto_low_get_cmd
    .global  proto_low_get_cmd
proto_low_get_cmd:
    clr     r25
    ldi     r24, 0xff
    sbic_clk
    ret
    ; read low idle byte nybble
    data_in
    andi    r18, 0x0f
    ; if cflg is low then set bit 4 of command
    sbis_cflg
    ori     r18, 0x10
    ; return result
    mov     r24, r18
    ret


    ; ----- proto_low_action -----
    ; void proto_low_action(void)
    ; confirm command with no value
    ; CL | WL | CH
    .section .text.proto_low_action
    .global  proto_low_action
proto_low_action:
    ; when we enter here the clk from master is already low
    ; now signal rak lo to the master. we are now in sync
    rak_lo
    ret


    ; ----- proto_low_end -----
    ; void proto_low_end(void)
    ; end an action or function call
    ; in: r24 = status
    ; out: -
    .section .text.proto_low_end
    .global  proto_low_end
proto_low_end:
    ; update status nybble
    andi    r24,0xf0
    ori     r24,0x0f
    mov     r18,r24
    data_out_prepare
    data_out_set
    ; command finishes with a last clk hi transition
    wait_clk_hi
    ; confirm it
    rak_hi
    ret


    ; ----- proto_low_write_word -----
    ; master wants to write a u16 function value
    ; u16 proto_low_write_word(void)
    ; in:  -
    ; out: r24,r25 = reg_num
    .section .text.proto_low_write_word
    .global  proto_low_write_word
proto_low_write_word:
    ; confirm to master we are ready
    rak_lo

    ; ddr: input
    wait_clk_hi
    ddr_in

    ; byte 1
    ; wait clk
    wait_clk_lo
    ; get data and save as result
    data_in
    mov     r25, r18

    ; byte 2
    ; wait clk
    wait_clk_hi
    data_in
    mov     r24, r18

    ; ddr: idle
    wait_clk_lo
    ddr_idle
    ret


    ; ----- proto_low_write_long -----
    ; master wants to write a u32 function value
    ; u32 proto_low_write_long(void)
    ; in:  -
    ; out: r22,r23,r24,r25 = value
    .section .text.proto_low_write_long
    .global  proto_low_write_long
proto_low_write_long:
    ; confirm to master we are ready
    rak_lo

    ; ddr: input
    wait_clk_hi
    ddr_in

    ; byte 1
    ; wait clk
    wait_clk_lo
    ; get data and save as result
    data_in
    mov     r25, r18

    ; byte 2
    ; wait clk
    wait_clk_hi
    data_in
    mov     r24, r18

    ; byte 3
    ; wait clk
    wait_clk_lo
    data_in
    mov     r23, r18

    ; byte 4
    ; wait clk
    wait_clk_hi
    data_in
    mov     r22, r18

    ; ddr: idle
    wait_clk_lo
    ddr_idle
    ret


    ; ----- proto_low_read_word -----
    ; master wants to read a u16 function value
    ; void proto_low_read_word(u16 v)
    ; in:  r24,r25 = read val
    ; out: -
    .section .text.proto_low_read_word
    .global  proto_low_read_word
proto_low_read_word:
    rak_lo

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
    ret


    ; ----- proto_low_read_long -----
    ; master wants to read a u32 function value
    ; void proto_low_read_long(u32 v)
    ; in:  r22,r23,r24,r25 = read val
    ; out: -
    .section .text.proto_low_read_long
    .global  proto_low_read_long
proto_low_read_long:
    rak_lo

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

    ; byte 3
    mov     r18, r23
    data_out_prepare
    wait_clk_lo
    data_out_set

    ; byte 4
    mov     r18, r22
    data_out_prepare
    wait_clk_hi
    data_out_set

    ; ddr: idle
    wait_clk_lo
    ddr_idle
    ret


    ; ----- proto_low_write_block -----
    ; master wants to write a message
    ; u16 proto_low_write_block(u16 max_words, u08 *buffer, u16 *chn_ext)
    ; in:  r25,r24 = max_words we can receive
    ; in:  r23,r22 = buffer ptr
    ; in:  r21,r20 = ptr to (extra << 8 | channel)
    ; out: r25,r24 = num_words received
    .section .text.proto_low_write_block
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

    ; check size: too large?
    cp      r24,r22
    cpc     r25,r23
    brsh    .wb_size_ok

    ; -> size invalid!
    ; signal with rak hi
    rak_hi

    ; end transmision
    ; clear result
    rjmp    .wb_empty

.wb_size_ok:
    ; empty size?
    or      r18,r23
    breq    .wb_empty

    ; save result size
    movw    r24,r22

    ; double size: convert to bytes
    add     r22,r22
    adc     r23,r23

    ; add start address: end address: r23:r22
    add     r22,r30
    adc     r23,r31

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
    ; u08 proto_low_read_block(u16 num_words, const u08 *buffer, u16 chn_ext)
    ; in:  r25,r24 = max_words we can receive
    ; in:  r23,r22 = buffer ptr
    ; in:  r21,r20 = extra << 8 | channel value
    ; out: r24 = 0=ok, 1=abort
    .section .text.proto_low_read_block
    .global  proto_low_read_block
proto_low_read_block:
    ; Z is buffer pointer
    movw    r30,r22

    ; save r20 (channel) -> r0
    ; warning: r20 gets globbered by first data_out* call!
    mov     r0,r20

    ; save old status value -> r22
    data_in
    mov     r22,r18

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
    breq    .rb_ok

    ; odd byte
    ld      r18,Z+
    data_out_prepare
    wait_clk_lo_abort_cfg_lo .rb_abort
    data_out_set
    debug_lo

    ; even byte
    ld      r18,Z+
    data_out_prepare
    wait_clk_hi
    data_out_set
    debug_hi

    rjmp    .rb_loop

.rb_abort:
    ; error result
    ldi     r24,1
    rjmp    .rb_end

.rb_ok:
    ; clear result=ok
    ldi     r24,0

.rb_end:
    ; ddr idle
    wait_clk_lo
    ddr_idle
    debug_lo

    ; restore old status value
    mov     r18,r22
    data_out_prepare
    data_out_set

    ; end sync
    wait_clk_hi
    rak_hi
    ret

    .end
