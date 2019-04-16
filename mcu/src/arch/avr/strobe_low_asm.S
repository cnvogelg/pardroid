#include "autoconf.h"
#include <avr/io.h>
#include "pario_pins.h"
#include "pario_pins.S"

    ; ----- strobe_low_init -----
    ; void strobe_low_init(void)
    ; setup parallel port lines for initial strobe wait
    .section .text.strobe_low_init
    .global  strobe_low_init
strobe_low_init:
    ; setup data port: input
    ddr_in
    ; set input
    ldi     r18,0xff
    data_out_prepare
    data_out_set
    ; setup control lines

    ; set OUTPUT levels: ACK=hi, BUSY=lo, SEL=hi
    sbi     _SFR_IO_ADDR(PAR_ACK_PORT), PAR_ACK_BIT
    cbi     _SFR_IO_ADDR(PAR_BUSY_PORT), PAR_BUSY_BIT
    sbi     _SFR_IO_ADDR(PAR_SELECT_PORT), PAR_SELECT_BIT
    ; pull ups on input
    sbi     _SFR_IO_ADDR(PAR_STROBE_PORT), PAR_STROBE_BIT
    sbi     _SFR_IO_ADDR(PAR_POUT_PORT), PAR_POUT_BIT

    ; DDR INPUT: strobe, pout(clk), select
    cbi     _SFR_IO_ADDR(PAR_STROBE_DDR), PAR_STROBE_BIT
    cbi     _SFR_IO_ADDR(PAR_POUT_DDR), PAR_POUT_BIT
    cbi     _SFR_IO_ADDR(PAR_SELECT_DDR), PAR_SELECT_BIT
    ; DDR OUTPUT: ack, busy
    sbi     _SFR_IO_ADDR(PAR_ACK_DDR), PAR_ACK_BIT
    sbi     _SFR_IO_ADDR(PAR_BUSY_DDR), PAR_BUSY_BIT

    ret

    ; ----- strobe_low_exit -----
    ; void strobe_low_exit(void)
    ; leave initial strobe phase
    .section .text.strobe_low_exit
    .global  strobe_low_exit
strobe_low_exit:
    ; all back to input
    cbi     _SFR_IO_ADDR(PAR_ACK_DDR), PAR_ACK_BIT
    cbi     _SFR_IO_ADDR(PAR_BUSY_DDR), PAR_BUSY_BIT
    cbi     _SFR_IO_ADDR(PAR_SELECT_DDR), PAR_SELECT_BIT
    ; sett all to one
    sbi     _SFR_IO_ADDR(PAR_BUSY_PORT), PAR_BUSY_BIT
    ret


    ; ----- strobe_low_get_data -----
    ; u08 strobe_low_get_data(void)
    ; out: r24 = data
    ; get data from parallel port
    .section .text.strobe_low_get_data
    .global  strobe_low_get_data
strobe_low_get_data:
    data_in
    mov     r24, r18
    ret


    ; ----- strobe_low_begin_send -----
    ; void strobe_low_begin_send(u08 data)
    ; in: r24 = data
    .section .text.strobe_low_begin_send
    .global  strobe_low_begin_send
strobe_low_begin_send:
    ; BUSY input (SEL, POUT are alread input)
    cbi     _SFR_IO_ADDR(PAR_BUSY_DDR), PAR_BUSY_BIT
    sbi     _SFR_IO_ADDR(PAR_BUSY_PORT), PAR_BUSY_BIT
    ; set data
    mov     r18,r24
    data_out_prepare
    data_out_set
    ; set data output
    ddr_out
    ret


    ; ----- strobe_low_end_send -----
    ; void strobe_low_end_send(void)
    .section .text.strobe_low_end_send
    .global  strobe_low_end_send
strobe_low_end_send:
    ; BUSY output again (lo)
    sbi     _SFR_IO_ADDR(PAR_BUSY_DDR), PAR_BUSY_BIT
    cbi     _SFR_IO_ADDR(PAR_BUSY_PORT), PAR_BUSY_BIT
    ; ddr input
    ddr_in
    ; set input
    ldi     r18,0xff
    data_out_prepare
    data_out_set
    ret


    ; ----- strobe_low_set_data -----
    ; void strobe_low_set_data(u08 data)
    ; in: r24 = data
    .section .text.strobe_low_set_data
    .global  strobe_low_set_data
strobe_low_set_data:
    mov      r18,r24
    data_out_prepare
    data_out_set
    ret
