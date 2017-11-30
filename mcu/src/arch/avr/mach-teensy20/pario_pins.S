    ; ----- 8 bit data port -----

    ; teensy20 uses full portb for data

    ; ddr_out
    ; set data direction to output
    ; in: -
    ; out: -
    ; regs: r18
    .macro ddr_out
    ldi     r18, 0xff                       ; 1
    out     _SFR_IO_ADDR(DDRC), r18         ; 1
    out     _SFR_IO_ADDR(DDRF), r18         ; 1
    .endm                                   ; = 3

    ; ddr_idle
    .macro ddr_idle
    ldi     r18, 0xf0                       ; 1
    out     _SFR_IO_ADDR(DDRF), r18         ; 1
    clr     r18                             ; 1
    out     _SFR_IO_ADDR(DDRC), r18         ; 1
    .endm                                   ; = 4

    ; ddr_in
    ; set data direction to input
    ; in: -
    ; out: -
    ; regs: r18
    .macro ddr_in
    clr     r18                             ; 1
    out     _SFR_IO_ADDR(DDRC), r18         ; 1
    out     _SFR_IO_ADDR(DDRF), r18         ; 1
    .endm                                   ; = 2

    ; data_out
    ; set data values on parallel port
    ; in: r18=value
    ; out: -
    ; regs: r19, r20
    .macro data_out_prepare
    mov     r19, r18                        ; 1
    swap    r19                             ; 1
    .endm
    .macro data_out_set
    out     _SFR_IO_ADDR(PORTC), r19        ; 1
    out     _SFR_IO_ADDR(PORTF), r18        ; 1
    .endm                                   ; = 4

    ; data_in
    ; read data values from parallel port
    ; in: -
    ; out: r18
    ; regs: -
    .macro data_in
    in      r19, _SFR_IO_ADDR(PINC)         ; 1
    in      r18, _SFR_IO_ADDR(PINF)         ; 1
    swap    r19                             ; 1
    or      r18, r19                        ; 1
    .endm                                   ; = 4
