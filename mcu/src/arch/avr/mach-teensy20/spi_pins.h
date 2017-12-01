/* SPI config Teensy 2.0

SPI_SS   = B0
SPI_MOSI = B2
SPI_MISO = B3
SPI_SCK  = B1

SPI_SS1  = D7

*/

#define SPI_SS_MASK     0x01
#define SPI_MOSI_MASK   0x04
#define SPI_MISO_MASK   0x08
#define SPI_SCK_MASK    0x02

#define SPI_SS1_DDR     DDRD
#define SPI_SS1_PORT    PORTD
#define SPI_SS1_MASK    0x80
