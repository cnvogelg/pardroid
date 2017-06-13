/*
    Parallel Port Connection
                     Arduino        Nano
    DATA 0 ... 5     PC 0 ... 5     same      IN/OUT
    DATA 6 ... 7     PD 6 ... 7     same      IN/OUT

    /STROBE          PD 2           PD3       IN (INT0 / INT1)
    SELECT           PD 3           PB1       OUT
    BUSY             PD 4           same      OUT
    POUT             PD 5           same      IN
    /ACK             PB 0           same      OUT
*/

// /STROBE (IN) (INT1) (D3)
#define PAR_STROBE_BIT          3
#define PAR_STROBE_MASK         _BV(PAR_STROBE_BIT)
#define PAR_STROBE_PORT         PORTD
#define PAR_STROBE_PIN          PIND
#define PAR_STROBE_DDR          DDRD

// SELECT (OUT) (B1)
#define PAR_SELECT_BIT          1
#define PAR_SELECT_MASK         _BV(PAR_SELECT_BIT)
#define PAR_SELECT_PORT         PORTB
#define PAR_SELECT_PIN          PINB
#define PAR_SELECT_DDR          DDRB

// BUSY (OUT) (D4)
#define PAR_BUSY_BIT            4
#define PAR_BUSY_MASK           _BV(PAR_BUSY_BIT)
#define PAR_BUSY_PORT           PORTD
#define PAR_BUSY_PIN            PIND
#define PAR_BUSY_DDR            DDRD

// POUT (IN) (D5)
#define PAR_POUT_BIT            5
#define PAR_POUT_MASK           _BV(PAR_POUT_BIT)
#define PAR_POUT_PORT           PORTD
#define PAR_POUT_PIN            PIND
#define PAR_POUT_DDR            DDRD

// /ACK (OUT) (D8)
#define PAR_ACK_BIT             0
#define PAR_ACK_MASK            _BV(PAR_ACK_BIT)
#define PAR_ACK_PORT            PORTB
#define PAR_ACK_PIN             PINB
#define PAR_ACK_DDR             DDRB

    ; ----- 8 bit data port -----

    ; Note: on Nano data port is split across two GPIO ports
    ; data port C: don't care about above bits (PC6, PC7)
    ; as they are not used (reset on PC6 is disabled)
    ; this saves us some instruction for OR/ANDin values

    ; ddr_out
    ; set data direction to output
    ; in: -
    ; out: -
    ; regs: r18, r19
    .macro ddr_out
    ldi     r18, 0x3f                       ; 1
    in      r19, _SFR_IO_ADDR(DDRD)         ; 1
    ori     r19, 0xc0                       ; 1
    out     _SFR_IO_ADDR(DDRC), r18         ; 1
    out     _SFR_IO_ADDR(DDRD), r19         ; 1
    .endm                                   ; = 5

    ; ddr_idle
    ; bits 0..4 out, bits 5..7 in
    .macro ddr_idle
    ldi     r18, 0x20                       ; 1
    in      r19, _SFR_IO_ADDR(DDRD)         ; 1
    ori     r19, 0xc0                       ; 1
    out     _SFR_IO_ADDR(DDRC), r18         ; 1
    out     _SFR_IO_ADDR(DDRD), r19         ; 1
    .endm                                   ; = 5

    ; ddr_in
    ; set data direction to input
    ; in: -
    ; out: -
    ; regs: r18, r19
    .macro ddr_in
    clr     r18                             ; 1
    in      r19, _SFR_IO_ADDR(DDRD)         ; 1
    andi    r19, 0x3f                       ; 1
    out     _SFR_IO_ADDR(DDRC), r18         ; 1
    out     _SFR_IO_ADDR(DDRD), r19         ; 1
    .endm                                   ; = 5

    ; data_out
    ; set data values on parallel port
    ; in: r18=value
    ; out: -
    ; regs: r19, r20
    .macro data_out_prepare
    mov     r19, r18                        ; 1
    in      r20, _SFR_IO_ADDR(PIND)         ; 1
    andi    r20, 0x3f                       ; 1
    andi    r19, 0xc0                       ; 1
    or      r19, r20                        ; 1
    .endm
    .macro data_out_set
    out     _SFR_IO_ADDR(PORTC), r18        ; 1
    out     _SFR_IO_ADDR(PORTD), r19        ; 1
    .endm                                   ; = 7 (!)

    ; data_in
    ; read data values from parallel port
    ; in: -
    ; out: r18
    ; regs: r19
    .macro data_in
    in      r18, _SFR_IO_ADDR(PINC)         ; 1
    in      r19, _SFR_IO_ADDR(PIND)         ; 1
    andi    r18, 0x3f                       ; 1
    andi    r19, 0xc0                       ; 1
    or      r18, r19                        ; 1
    .endm                                   ; = 5


