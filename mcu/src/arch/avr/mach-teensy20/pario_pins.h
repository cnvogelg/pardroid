/*
    Parallel Port Connection (Teensy 2.0)
                      AVR
    DATA0            PF 0
    DATA1            PF 1
    DATA2            PC 6
    DATA3            PC 7
    DATA4            PF 4
    DATA5            PF 5
    DATA6            PF 6
    DATA7            PF 7

    /STROBE          PE 6           IN (INT6)
    SELECT           PB 7           IN
    POUT             PB 6           IN
    BUSY             PB 5           OUT
    /ACK             PB 4           OUT
*/

// /STROBE (IN)
#define PAR_STROBE_BIT          6
#define PAR_STROBE_MASK         _BV(PAR_STROBE_BIT)
#define PAR_STROBE_PORT         PORTE
#define PAR_STROBE_PIN          PINE
#define PAR_STROBE_DDR          DDRE

#define PAR_STROBE_INT          INT6
#define PAR_STROBE_VECT         INT6_vect
#define PAR_STROBE_ISC          ISC61
#define PAR_STROBE_EICR         EICRB

// SELECT (IN) (INT1)
#define PAR_SELECT_BIT          7
#define PAR_SELECT_MASK         _BV(PAR_SELECT_BIT)
#define PAR_SELECT_PORT         PORTB
#define PAR_SELECT_PIN          PINB
#define PAR_SELECT_DDR          DDRB

// POUT (IN)
#define PAR_POUT_BIT            6
#define PAR_POUT_MASK           _BV(PAR_POUT_BIT)
#define PAR_POUT_PORT           PORTB
#define PAR_POUT_PIN            PINB
#define PAR_POUT_DDR            DDRB

// BUSY (OUT)
#define PAR_BUSY_BIT            5
#define PAR_BUSY_MASK           _BV(PAR_BUSY_BIT)
#define PAR_BUSY_PORT           PORTB
#define PAR_BUSY_PIN            PINB
#define PAR_BUSY_DDR            DDRB

// /ACK (OUT)
#define PAR_ACK_BIT             4
#define PAR_ACK_MASK            _BV(PAR_ACK_BIT)
#define PAR_ACK_PORT            PORTB
#define PAR_ACK_PIN             PINB
#define PAR_ACK_DDR             DDRB
