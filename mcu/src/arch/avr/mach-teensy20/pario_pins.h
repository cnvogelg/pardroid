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

#define PAR_STROBE_INT          INT0
#define PAR_STROBE_VECT         INT0_vect
#define PAR_STROBE_ISC          ISC01

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
