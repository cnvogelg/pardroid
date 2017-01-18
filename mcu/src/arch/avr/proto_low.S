#include "autoconf.h"
#include <avr/io.h>
#include "pario_pins.S"

    ; some convenience macros for the control signals
    ; CLK = POUT
    ; RAK = BUSY

    .macro sbic_clk
    sbic    _SFR_IO_ADDR(PAR_POUT_PORT), PAR_POUT_BIT
    .endm

    .macro sbis_clk
    sbis    _SFR_IO_ADDR(PAR_POUT_PORT), PAR_POUT_BIT
    .endm

    .macro wait_clk_lo
l\@: sbic    _SFR_IO_ADDR(PAR_POUT_PORT), PAR_POUT_BIT
    rjmp     l\@
    .endm

    .macro wait_clk_hi
l\@: sbis    _SFR_IO_ADDR(PAR_POUT_PORT), PAR_POUT_BIT
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

    .section    text

    ; void pario_init(void)
    ; setup parallel port lines
    .global  proto_low_init
proto_low_init:
    ; setup data port: INPUT
    ddr_in
    ; setup control lines
    ; DDR INPUT: strobe, select, pout
    cbi     _SFR_IO_ADDR(PAR_STROBE_DDR), PAR_STROBE_BIT
    cbi     _SFR_IO_ADDR(PAR_SELECT_DDR), PAR_SELECT_BIT
    cbi     _SFR_IO_ADDR(PAR_POUT_DDR), PAR_POUT_BIT
    ; DDR OUTPUT: ack, busy
    sbi     _SFR_IO_ADDR(PAR_ACK_DDR), PAR_ACK_BIT
    sbi     _SFR_IO_ADDR(PAR_BUSY_DDR), PAR_BUSY_BIT
    ; set OUTPUT levels
    ack_hi
    rak_hi
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
    rak_lo
    wait_clk_hi
    rak_hi
    ret


    ; CL | WL | ( DO | CH )+ | RS | CH
    .global  proto_low_test_write
proto_low_test_write:
    ; confirm to master we are ready
    rak_lo

    ; byte 1
    ; wait clk
    wait_clk_hi
    ; get data and save as result
    data_in
    mov     r24, r18

    ; byte 2
    ; wait clk
    wait_clk_lo
    data_in
    mov     r25, r18

    ; end sync
    wait_clk_hi
    rak_hi
    ret


    ; CL | WL | XI | ( CH | DI )+ | RS | CH | XO
    .global  proto_low_test_read
proto_low_test_read:
    rak_lo

    ; byte 1
    wait_clk_hi
    ddr_out
    mov     r18, r25
    data_out

    ; byte 2
    wait_clk_lo
    mov     r18, r24
    data_out

    ; end sync
    wait_clk_hi
    ddr_in
    rak_hi
    ret

