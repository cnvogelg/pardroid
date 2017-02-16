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
    .macro debug_lo
    cbi     _SFR_IO_ADDR(PAR_SELECT_PORT), PAR_SELECT_BIT
    .endm
    .macro debug_hi
    sbi     _SFR_IO_ADDR(PAR_SELECT_PORT), PAR_SELECT_BIT
    .endm
#else
    .macro debug_lo
    .endm
    .macro debug_hi
    .endm
#endif

    .section .text

    ; void pario_init(void)
    ; setup parallel port lines
    .global  proto_low_init
proto_low_init:
    ; setup data port: INPUT
    ddr_in
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


    ; u08 pario_get_cmd(void)
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


    ; void pario_cmd_ping(void)
    ; confirm ping command
    ; CL | WL | CH
    .global  proto_low_ping
proto_low_ping:
    ; when we enter here the clk from master is already low
    ; now signal rak lo to the master. we are now in sync
    rak_lo

    ; ping only finishes with a last clk hi transition
    wait_clk_hi
    ; confirm it
    rak_hi
    ret


    ; u16 proto_low_test_write(void)
    ; r24,r25 = wrote val
    .global  proto_low_test_write
proto_low_test_write:
    ; confirm to master we are ready
    rak_lo

    ; byte 1
    ; wait clk
    wait_clk_hi
    ; get data and save as result
    data_in
    ; store in buffer
    mov     r25, r18
    debug_hi

    ; byte 2
    ; wait clk
    wait_clk_lo
    data_in
    mov     r24, r18
    debug_lo

    ; end sync
    wait_clk_hi
    rak_hi
    ret


    ; void proto_low_test_read(u16 v)
    ; r24,r25 = read val
    .global  proto_low_test_read
proto_low_test_read:
    rak_lo

    ; data: write
    wait_clk_hi
    ddr_out
    debug_hi

    ; byte 1
    mov     r18, r25
    data_out_prepare
    wait_clk_lo
    data_out_set
    debug_lo

    ; byte 2
    mov     r18, r24
    data_out_prepare
    wait_clk_hi
    data_out_set
    debug_hi

    ; data: read
    wait_clk_lo
    ddr_in
    debug_lo

    ; end sync
    wait_clk_hi
    rak_hi
    ret

    .end

