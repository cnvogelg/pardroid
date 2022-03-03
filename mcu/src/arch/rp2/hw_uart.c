#include "hardware/uart.h"

#include "autoconf.h"
#include "types.h"

#include "hw_uart.h"

void hw_uart_init(void)
{
    uart_init(uart0, CONFIG_BAUD_RATE);
}

void hw_uart_send(u08 data)
{
    uart_putc(uart0, data);
}
