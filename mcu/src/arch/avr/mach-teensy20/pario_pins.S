/*
    Parallel Port Connection (Teensy 2.0)
                      AVR
    DATA 0 ... 7     PB 0 ... 7     IN/OUT

    /STROBE          PF 0           IN (INT0)
    SELECT           PF 6           IN
    POUT             PF 5           IN
    BUSY             PF 4           OUT
    /ACK             PF 1           OUT
*/

// /STROBE (IN) (INT0)
#define PAR_STROBE_BIT          0
#define PAR_STROBE_MASK         _BV(PAR_STROBE_BIT)
#define PAR_STROBE_PORT         PORTF
#define PAR_STROBE_PIN          PINF
#define PAR_STROBE_DDR          DDRF

// SELECT (IN) (INT1)
#define PAR_SELECT_BIT          6
#define PAR_SELECT_MASK         _BV(PAR_SELECT_BIT)
#define PAR_SELECT_PORT         PORTF
#define PAR_SELECT_PIN          PINF
#define PAR_SELECT_DDR          DDRF

// POUT (IN)
#define PAR_POUT_BIT            5
#define PAR_POUT_MASK           _BV(PAR_POUT_BIT)
#define PAR_POUT_PORT           PORTF
#define PAR_POUT_PIN            PINF
#define PAR_POUT_DDR            DDRF

// BUSY (OUT)
#define PAR_BUSY_BIT            4
#define PAR_BUSY_MASK           _BV(PAR_BUSY_BIT)
#define PAR_BUSY_PORT           PORTF
#define PAR_BUSY_PIN            PINF
#define PAR_BUSY_DDR            DDRF

// /ACK (OUT)
#define PAR_ACK_BIT             1
#define PAR_ACK_MASK            _BV(PAR_ACK_BIT)
#define PAR_ACK_PORT            PORTF
#define PAR_ACK_PIN             PINF
#define PAR_ACK_DDR             DDRF

    ; ----- 8 bit data port -----

    ; teensy20 uses full portb for data

    ; ddr_out
    ; set data direction to output
    ; in: -
    ; out: -
    ; regs: r18
    .macro ddr_out
    ldi     r18, 0xff                       ; 1
    out     _SFR_IO_ADDR(DDRB), r18         ; 1
    .endm                                   ; = 2

    ; ddr_idle
    .macro ddr_idle
    clr     r18                             ; 1
    out     _SFR_IO_ADDR(DDRB), r18         ; 1
    .endm                                   ; = 2

    ; ddr_in
    ; set data direction to input
    ; in: -
    ; out: -
    ; regs: r18
    .macro ddr_in
    clr     r18                             ; 1
    out     _SFR_IO_ADDR(DDRB), r18         ; 1
    .endm                                   ; = 2

    ; data_out
    ; set data values on parallel port
    ; in: r18=value
    ; out: -
    ; regs: -
    .macro data_out_prepare
    .endm
    .macro data_out_set
    out     _SFR_IO_ADDR(PORTB), r18        ; 1
    .endm                                   ; = 1

    ; data_in
    ; read data values from parallel port
    ; in: -
    ; out: r18
    ; regs: -
    .macro data_in
    in      r18, _SFR_IO_ADDR(PINB)         ; 1
    .endm                                   ; = 1
