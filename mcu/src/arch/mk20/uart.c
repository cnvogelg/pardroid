#include "kinetis.h"

#include "autoconf.h"
#include "types.h"

#include "uart.h"

// calc baud divisor
#define BAUD CONFIG_BAUD_RATE
#define BAUD2DIV(baud)  (((F_CPU * 2) + ((baud) >> 1)) / (baud))

void uart_init(void)
{
    // turn on UART clock
    SIM_SCGC4 |= SIM_SCGC4_UART0;

    // pin config RX/TX aka PIN0/PIN1 aka C16, C17
    PORTB_PCR16 = PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_PFE | PORT_PCR_MUX(3); // RX (in)
    PORTB_PCR17 = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3); // TX (out)

    // setup divisor
    const uint32_t divisor = BAUD2DIV(CONFIG_BAUD_RATE);
    UART0_BDH = (divisor >> 13) & 0x1F;
    UART0_BDL = (divisor >> 5) & 0xFF;
    UART0_C4 = divisor & 0x1F;

    // no FIFO
    UART0_C1 = 0;
    UART0_PFIFO = 0;

    // enable
    UART0_C2 = UART_C2_TE | UART_C2_RE;
}

void uart_send(u08 data)
{
    // wait for last transfer complete
    while(!(UART0_S1 & UART_S1_TC));

    // send data
    UART0_D = data;
}
