// Microchip ENC28J60 Ethernet Interface Driver
// Author: Guido Socher
// Copyright: GPL V2
//
// Based on the enc28j60.c file from the AVRlib library by Pascal Stang.
// For AVRlib See http://www.procyonengineering.com/
// Used with explicit permission of Pascal Stang.
//
// 2010-05-20 <jc@wippler.nl>
//
// Adjusted by Christian Vogelgsaang to be Arduino-free C code

#include "autoconf.h"
#include "types.h"
#include "timer.h"
#include "spi.h"

#include "enc28j60.h"
#include "enc28j60_regs.h"

#if CONFIG_DRIVER_ENC28J60_SPI_CS == 0
#define spi_enable_eth()   spi_enable_cs0()
#define spi_disable_eth()  spi_disable_cs0()
#elif CONFIG_DRIVER_ENC28J60_SPI_CS == 1
#define spi_enable_eth()   spi_enable_cs1()
#define spi_disable_eth()  spi_disable_cs1()
#else
#error invalid CONFIG_DRIVER_ENC28J60_SPI_CS
#endif


static u08 Enc28j60Bank;
static u16 gNextPacketPtr;
static u08 is_full_duplex;

// ----- basic SPI ops -----

static uint8_t readOp (uint8_t op, uint8_t address) {
    spi_enable_eth();
    spi_out(op | (address & ADDR_MASK));
    if (address & 0x80)
        spi_out(0x00);
    uint8_t result = spi_in();
    spi_disable_eth();
    return result;
}

static void writeOp (uint8_t op, uint8_t address, uint8_t data) {
    spi_enable_eth();
    spi_out(op | (address & ADDR_MASK));
    spi_out(data);
    spi_disable_eth();
}

static void readBuf(uint16_t len, uint8_t* data) {
    spi_enable_eth();
    spi_out(ENC28J60_READ_BUF_MEM);
    while (len--) {
        *data++ = spi_in();
    }
    spi_disable_eth();
}

static void writeBuf(uint16_t len, const uint8_t *data) {
    spi_enable_eth(),
    spi_out(ENC28J60_WRITE_BUF_MEM);
    while(len--) {
      spi_out(*data++);
    }
    spi_disable_eth();
}

// ----- high level ops -----

static void SetBank (uint8_t address) {
    if ((address & BANK_MASK) != Enc28j60Bank) {
        writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_BSEL1|ECON1_BSEL0);
        Enc28j60Bank = address & BANK_MASK;
        writeOp(ENC28J60_BIT_FIELD_SET, ECON1, Enc28j60Bank>>5);
    }
}

static uint8_t readRegByte (uint8_t address) {
    SetBank(address);
    return readOp(ENC28J60_READ_CTRL_REG, address);
}

#if 0
static uint16_t readReg(uint8_t address) {
	return readRegByte(address) + (readRegByte(address+1) << 8);
}
#endif

static void writeRegByte (uint8_t address, uint8_t data) {
    SetBank(address);
    writeOp(ENC28J60_WRITE_CTRL_REG, address, data);
}

static void writeReg(uint8_t address, uint16_t data) {
    writeRegByte(address, data);
    writeRegByte(address + 1, data >> 8);
}

static uint16_t readPhyByte (uint8_t address) {
    writeRegByte(MIREGADR, address);
    writeRegByte(MICMD, MICMD_MIIRD);
    while (readRegByte(MISTAT) & MISTAT_BUSY)
        ;
    writeRegByte(MICMD, 0x00);
    return readRegByte(MIRD+1);
}

static void writePhy (uint8_t address, uint16_t data) {
    writeRegByte(MIREGADR, address);
    writeReg(MIWR, data);
    while (readRegByte(MISTAT) & MISTAT_BUSY)
        ;
}

// ---------- init ----------

// Functions to enable/disable broadcast filter bits
// With the bit set, broadcast packets are filtered.
INLINE void enc28j60_enable_broadcast ( void )
{
  writeRegByte(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN/*|ERXFCON_PMEN*/|ERXFCON_BCEN);
}

INLINE void enc28j60_disable_broadcast ( void )
{
  writeRegByte(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN/*|ERXFCON_PMEN*/);
}

u08 enc28j60_init(u08 *rev_ret)
{
  *rev_ret = 0;

  // soft reset cpu
  writeOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
  timer_delay(2); // errata B7/2

  // wait or error
  u16 count = 0;
  while (!readOp(ENC28J60_READ_CTRL_REG, ESTAT) & ESTAT_CLKRDY) {
    count ++;
    if(count == 0xfff) {
      return ENC28J60_RESULT_NOT_FOUND;
    }
  }

  // return rev
  u08 rev = readRegByte(EREVID);
  // microchip forgot to step the number on the silcon when they
  // released the revision B7. 6 is now rev B7. We still have
  // to see what they do when they release B8. At the moment
  // there is no B8 out yet
  if (rev > 5) ++rev;
  *rev_ret = rev;

  return ENC28J60_RESULT_OK;
}

u08 enc28j60_setup(const u08 macaddr[6], u08 flags)
{
  // set packet pointers
  gNextPacketPtr = RXSTART_INIT;
  writeReg(ERXST, RXSTART_INIT);
  writeReg(ERXRDPT, RXSTART_INIT);
  writeReg(ERXND, RXSTOP_INIT);
  writeReg(ETXST, TXSTART_INIT);
  writeReg(ETXND, TXSTOP_INIT);

  // set packet filter
  if(flags & ENC28J60_FLAGS_BROADCAST) {
    enc28j60_enable_broadcast(); // change to add ERXFCON_BCEN recommended by epam
  } else {
    enc28j60_disable_broadcast(); // change to add ERXFCON_BCEN recommended by epam
  }
  is_full_duplex = (flags & ENC28J60_FLAGS_FULL_DUPLEX);

  // BIST pattern generator?
  writeReg(EPMM0, 0x303f);
  writeReg(EPMCS, 0xf7f9);

  // MAC init (with flow control)
  writeRegByte(MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
  writeRegByte(MACON2, 0x00);
  u08 mac3val = MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN;
  if(is_full_duplex) {
    mac3val |= MACON3_FULDPX;
  }
  writeRegByte(MACON3, mac3val);

  if(is_full_duplex) {
    writeRegByte(MABBIPG, 0x15);
    writeReg(MAIPG, 0x0012);
  } else {
    writeRegByte(MABBIPG, 0x12);
    writeReg(MAIPG, 0x0C12);
  }
  writeReg(MAMXFL, MAX_FRAMELEN);

  // PHY init
  if(is_full_duplex) {
    writePhy(PHCON1, PHCON1_PDPXMD);
    writePhy(PHCON2, 0);
  } else {
    writePhy(PHCON1, 0);
    writePhy(PHCON2, PHCON2_HDLDIS);
  }

  // prepare flow control
  writeReg(EPAUS, 20 * 100); // 100ms

  // set mac
  writeRegByte(MAADR5, macaddr[0]);
  writeRegByte(MAADR4, macaddr[1]);
  writeRegByte(MAADR3, macaddr[2]);
  writeRegByte(MAADR2, macaddr[3]);
  writeRegByte(MAADR1, macaddr[4]);
  writeRegByte(MAADR0, macaddr[5]);

  SetBank(ECON1);
  writeOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE);
  return ENC28J60_RESULT_OK;
}

void enc28j60_enable(void)
{
  SetBank(ECON1);
  writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
}

void enc28j60_disable(void)
{
  SetBank(ECON1);
  writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_RXEN);
}

// ---------- control ----------

void enc28j60_flow_control(u08 on)
{
  u08 flag;
  if(is_full_duplex) {
    flag = on ? 2 : 0;
  } else {
    flag = on ? 1 : 0;
  }
  writeRegByte(EFLOCON, flag);
}

// ---------- status ----------

u08 enc28j60_is_link_up(void)
{
  return (readPhyByte(PHSTAT2) >> 2) & 1;
}

#if 0
static u08 enc28j60_get_status( void )
{
  u08 val = readRegByte(EIR);
  if(val & EIR_TXERIF) {
    writeOp(ENC28J60_BIT_FIELD_CLR, EIR, EIR_TXERIF);
  }
  if(val & EIR_RXERIF) {
    writeOp(ENC28J60_BIT_FIELD_CLR, EIR, EIR_RXERIF);
  }
  return val & 3;
}
#endif

// ---------- send ----------

void enc28j60_send(const u08 *data, u16 size)
{
  // prepare tx buffer write
  writeReg(EWRPT, TXSTART_INIT);
  writeOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);

  // fill buffer
  writeBuf(size, data);

  // wait for tx ready
  while (readOp(ENC28J60_READ_CTRL_REG, ECON1) & ECON1_TXRTS) {
    if (readRegByte(EIR) & EIR_TXERIF) {
        writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
        writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
    }
  }

  // initiate send
  writeReg(ETXND, TXSTART_INIT+size);
  writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}

void enc28j60_send_begin(void)
{
  // prepare tx buffer write
  writeReg(EWRPT, TXSTART_INIT);
  writeOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
}

void enc28j60_send_seek(u16 abs_pos)
{
  writeReg(EWRPT, TXSTART_INIT + 1 + abs_pos); // skip 1 control byte
}

void enc28j60_send_data(const u08 *data, u16 size)
{
  writeBuf(size, data);
}

void enc28j60_send_end(u16 total_size)
{
  // wait for tx ready
  while (readOp(ENC28J60_READ_CTRL_REG, ECON1) & ECON1_TXRTS) {
    if (readRegByte(EIR) & EIR_TXERIF) {
        writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
        writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);
    }
  }

  // initiate send
  writeReg(ETXND, TXSTART_INIT + total_size);
  writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}

// ---------- recv ----------

INLINE void next_pkt(void)
{
  if (gNextPacketPtr - 1 > RXSTOP_INIT)
      writeReg(ERXRDPT, RXSTOP_INIT);
  else
      writeReg(ERXRDPT, gNextPacketPtr - 1);
  writeOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
}

static u08 read_hdr(u16 *got_size)
{
  struct {
      uint16_t nextPacket;
      uint16_t byteCount;
      uint16_t status;
  } header;

  readBuf(sizeof header, (uint8_t*) &header);

  gNextPacketPtr  = header.nextPacket;
  *got_size = header.byteCount - 4; //remove the CRC count
  return header.status;
}

u08 enc28j60_recv(u08 *data, u16 max_size, u16 *got_size)
{
  writeReg(ERDPT, gNextPacketPtr);

  // read chip's packet header
  u08 status = read_hdr(got_size);

  // was a receive error?
  if ((status & 0x80)==0) {
    next_pkt();
    return ENC28J60_RESULT_RX_ERROR;
  }

  // check size
  u16 len = *got_size;
  u08 result = ENC28J60_RESULT_OK;
  if(len > max_size) {
    len = max_size;
    result = ENC28J60_RESULT_RX_TOO_LARGE;
  }

  // read packet
  readBuf(len, data);

  next_pkt();
  return result;
}

u08 enc28j60_recv_begin(u16 *got_size)
{
  writeReg(ERDPT, gNextPacketPtr);

  // read chip's packet header
  u08 status = read_hdr(got_size);

  // was a receive error?
  if ((status & 0x80)==0) {
    next_pkt();
    return ENC28J60_RESULT_RX_ERROR;
  }

  return ENC28J60_RESULT_OK;
}

void enc28j60_recv_seek(u16 abs_pos)
{
  writeReg(ERDPT, gNextPacketPtr + 6 + abs_pos); // skip 6 header bytes
}

void enc28j60_recv_data(u08 *data, u16 size)
{
  readBuf(size, data);
}

void enc28j60_recv_end(void)
{
  next_pkt();
}

// ---------- has_recv ----------

u08 enc28j60_get_pending_packets(void)
{
  return readRegByte(EPKTCNT);
}

#if 0
// Contributed by Alex M. Based on code from: http://blog.derouineau.fr
//                  /2011/07/putting-enc28j60-ethernet-controler-in-sleep-mode/
void enc28j60_power_down( void )
{
    writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_RXEN);
    while(readRegByte(ESTAT) & ESTAT_RXBUSY);
    while(readRegByte(ECON1) & ECON1_TXRTS);
    writeOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_VRPS);
    writeOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PWRSV);
}

void enc28j60_power_up( void )
{
    writeOp(ENC28J60_BIT_FIELD_CLR, ECON2, ECON2_PWRSV);
    while(!readRegByte(ESTAT) & ESTAT_CLKRDY);
    writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
}

uint8_t enc28j60_do_BIST ( void )
{
	#define RANDOM_FILL		0b0000
	#define ADDRESS_FILL	0b0100
	#define PATTERN_SHIFT	0b1000
	#define RANDOM_RACE		0b1100

// init
    spi_disable_eth();

    writeOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
    _delay_ms(2); // errata B7/2
    while (!readOp(ENC28J60_READ_CTRL_REG, ESTAT) & ESTAT_CLKRDY) ;

	// now we can start the memory test
	uint16_t macResult;
	uint16_t bitsResult;

	// clear some of the registers registers
    writeRegByte(ECON1, 0);
	writeReg(EDMAST, 0);

	// Set up necessary pointers for the DMA to calculate over the entire memory
	writeReg(EDMAND, 0x1FFFu);
	writeReg(ERXND, 0x1FFFu);

	// Enable Test Mode and do an Address Fill
	SetBank(EBSTCON);
	writeRegByte(EBSTCON, EBSTCON_TME | EBSTCON_BISTST | ADDRESS_FILL);

	// wait for BISTST to be reset, only after that are we actually ready to
	// start the test
	// this was undocumented :(
    while (readOp(ENC28J60_READ_CTRL_REG, EBSTCON) & EBSTCON_BISTST);
	writeOp(ENC28J60_BIT_FIELD_CLR, EBSTCON, EBSTCON_TME);

	// now start the actual reading an calculating the checksum until the end is
	// reached
	writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_DMAST | ECON1_CSUMEN);
	SetBank(EDMACS);
	while(readOp(ENC28J60_READ_CTRL_REG, ECON1) & ECON1_DMAST);
	macResult = readReg(EDMACS);
	bitsResult = readReg(EBSTCS);
	// Compare the results
	// 0xF807 should always be generated in Address fill mode
	if ((macResult != bitsResult) || (bitsResult != 0xF807)) {
		return 0;
	}
	// reset test flag
	writeOp(ENC28J60_BIT_FIELD_CLR, EBSTCON, EBSTCON_TME);


	// Now start the BIST with random data test, and also keep on swapping the
	// DMA/BIST memory ports.
	writeRegByte(EBSTSD, 0b10101010 /* | millis()*/);
	writeRegByte(EBSTCON, EBSTCON_TME | EBSTCON_PSEL | EBSTCON_BISTST | RANDOM_FILL);


	// wait for BISTST to be reset, only after that are we actually ready to
	// start the test
	// this was undocumented :(
    while (readOp(ENC28J60_READ_CTRL_REG, EBSTCON) & EBSTCON_BISTST);
	writeOp(ENC28J60_BIT_FIELD_CLR, EBSTCON, EBSTCON_TME);


	// now start the actual reading an calculating the checksum until the end is
	// reached
	writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_DMAST | ECON1_CSUMEN);
	SetBank(EDMACS);
	while(readOp(ENC28J60_READ_CTRL_REG, ECON1) & ECON1_DMAST);

	macResult = readReg(EDMACS);
	bitsResult = readReg(EBSTCS);
	// The checksum should be equal
	return macResult == bitsResult;
}
#endif
