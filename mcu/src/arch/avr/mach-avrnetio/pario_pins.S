
/*
    Parallel Port Connection (AVR Net IO board)
                      AVR
    DATA 0 ... 7     PC 0 ... 7     IN/OUT

    /STROBE          PD 2           IN (INT0)
    SELECT           PA 3           IN
    POUT             PA 2           IN
    BUSY             PA 1           OUT
    /ACK             PA 0           OUT
*/

// /STROBE (IN) (INT0)
#define PAR_STROBE_BIT          2
#define PAR_STROBE_MASK         _BV(PAR_STROBE_BIT)
#define PAR_STROBE_PORT         PORTD
#define PAR_STROBE_PIN          PIND
#define PAR_STROBE_DDR          DDRD

// SELECT (IN) (INT1)
#define PAR_SELECT_BIT          3
#define PAR_SELECT_MASK         _BV(PAR_SELECT_BIT)
#define PAR_SELECT_PORT         PORTA
#define PAR_SELECT_PIN          PINA
#define PAR_SELECT_DDR          DDRA

// POUT (IN)
#define PAR_POUT_BIT            2
#define PAR_POUT_MASK           _BV(PAR_POUT_BIT)
#define PAR_POUT_PORT           PORTA
#define PAR_POUT_PIN            PINA
#define PAR_POUT_DDR            DDRA

// BUSY (OUT)
#define PAR_BUSY_BIT            1
#define PAR_BUSY_MASK           _BV(PAR_BUSY_BIT)
#define PAR_BUSY_PORT           PORTA
#define PAR_BUSY_PIN            PINA
#define PAR_BUSY_DDR            DDRA

// /ACK (OUT)
#define PAR_ACK_BIT             0
#define PAR_ACK_MASK            _BV(PAR_ACK_BIT)
#define PAR_ACK_PORT            PORTA
#define PAR_ACK_PIN             PINA
#define PAR_ACK_DDR             DDRA

    ; ----- 8 bit data port -----

    ; avrnetio uses full portc for data

    ; ddr_out
    ; set data direction to output
    ; in: -
    ; out: -
    ; regs: r18
    .macro ddr_out
    ldi     r18, 0xff                       ; 1
    out     _SFR_IO_ADDR(DDRC), r18         ; 1
    .endm                                   ; = 2

    ; ddr_in
    ; set data direction to input
    ; in: -
    ; out: -
    ; regs: r18
    .macro ddr_in
    clr     r18                             ; 1
    out     _SFR_IO_ADDR(DDRC), r18         ; 1
    .endm                                   ; = 2

    ; data_out
    ; set data values on parallel port
    ; in: r18=value
    ; out: -
    ; regs: -
    .macro data_out_prepare
    .endm
    .macro data_out_set
    out     _SFR_IO_ADDR(PORTC), r18        ; 1
    .endm                                   ; = 1

    ; data_in
    ; read data values from parallel port
    ; in: -
    ; out: r18
    ; regs: -
    .macro data_in
    in      r18, _SFR_IO_ADDR(PINC)         ; 1
    .endm                                   ; = 1
