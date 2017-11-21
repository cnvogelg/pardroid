/*
 * sd card routines
 *
 * heavily inspired by: www.sd2iec.de
 *  sd2iec - SD/MMC to Commodore serial bus interface/controller
 *           Copyright (C) 2007-2017  Ingo Korb <ingo@akana.de>
 *
 * also see:
 * - http://bikealive.nl/sd-v2-initialization.html
 * - https://yannik520.github.io/sdio.html
 */

#include "autoconf.h"
#include "types.h"
#include "timer.h"
#include "spi.h"
#include "crc.h"
#include "system.h"

#define DEBUG CONFIG_DEBUG_DRIVER_SDCARD

#include "debug.h"

#include "sdcard.h"
#include "sdcard_defs.h"

#ifdef CONFIG_DRIVER_SDCARD_CRC
#define USE_CRC
#endif

#if CONFIG_DRIVER_SDCARD_SPI_CS == 0
#define spi_enable_cs()   spi_enable_cs0()
#define spi_disable_cs()  spi_disable_cs0()
#elif CONFIG_DRIVER_SDCARD_SPI_CS == 1
#define spi_enable_cs()   spi_enable_cs1()
#define spi_disable_cs()  spi_disable_cs1()
#else
#error invalid CONFIG_DRIVER_SDCARD_SPI_CS
#endif

#define SD_INIT_TIMEOUT     2000
#define SD_WRITE_TIMEOUT    600
#define SD_TOKEN_TIMEOUT    100
#define SD_BUSY_TIMEOUT     500
#define SD_AUTO_RETRIES     10
#define SD_INIT_RETRIES     3

static u08 card_type = SDCARD_TYPE_NONE;

static u32 read_u32(void)
{
  u08 buf[4];
  buf[3] = spi_in();
  buf[2] = spi_in();
  buf[1] = spi_in();
  buf[0] = spi_in();
  return (u32)buf[3] << 24 | (u32)buf[2] << 16 | (u32)buf[1] << 8 | (u32)buf[0];
}

static void end_command(void)
{
  spi_disable_cs();
  spi_out(0xff);
  spi_out(0xff);
  spi_out(0xff);
}

static u08 begin_command(u08 cmd, u32 arg)
{
  // form message
  uint8_t buf[6];
  buf[0] = (uint8_t)0x40U | cmd;
  buf[1] = (uint8_t)(arg >> 24U);
  buf[2] = (uint8_t)(arg >> 16U);
  buf[3] = (uint8_t)(arg >> 8U);
  buf[4] = (uint8_t)arg;
  // add CRC
  buf[5] = crc7(buf, 5);

  u08 errors = 0;
  u08 res;
  while(errors < SD_AUTO_RETRIES) {
    spi_enable_cs();

    DC('['); DB(cmd); DC('+'); DL(arg); DC('@'); DB(buf[5]);
    for(u08 i=0;i<6;i++) {
      spi_out(buf[i]);
    }

    timer_ms_t t0 = timer_millis();
    while(1) {
      res = spi_in();
      DB(res); DC(',');
      if((res & 0x80) == 0) {
        break;
      }
      if(timer_millis_timed_out(t0, SD_WRITE_TIMEOUT)) {
        break;
      }
      system_wdt_reset();
    }

    if(res & STATUS_CRC_ERROR) {
      DC('X');
      end_command();
      errors++;
    } else {
      break;
    }
  }
  DC('='); DB(res); DC(']');
  return res;
}

static u08 enter_idle(void)
{
  for(u08 retries=0;retries<SD_INIT_RETRIES;retries++) {
    // must supply min of 74 clock cycles with CS high.
    DC('I');
    spi_disable_cs();
    for (uint8_t i = 0; i < 10; i++) {
      spi_out(0XFF);
    }

    // command to go idle in SPI mode
    u08 res = begin_command(CMD0_GO_IDLE_STATE, 0);
    end_command();
    if(res & 0x80) {
      return SDCARD_RESULT_FAILED_IDLE;
    }
    // reach idle state
    if(res == STATUS_IDLE_STATE) {
      return SDCARD_RESULT_OK;
    }
  }
  // failed retries
  return SDCARD_RESULT_FAILED_IDLE;
}

static u08 send_if_cond(void)
{
  u08 res = begin_command(CMD8_SEND_IF_COND, 0x1aa);
  if(res == 1) {
    u32 arg = read_u32();
    end_command();
    DC('?'); DL(arg);
    if(((arg >> 8) & 0xf) != 1) {
      return SDCARD_RESULT_FAILED_VOLTAGE;
    }
  }
  // do not abort on command failure here as some card don't support it
  return SDCARD_RESULT_OK;
}

static u08 sd_send_op_cond(u08 *is_sd)
{
  timer_ms_t t0 = timer_millis();
  while(1) {
    /* APP_CMD */
    u08 res = begin_command(CMD_APP, 0);
    end_command();
    if(res != 1) {
      /* MMC card don't support it */
      *is_sd = SDCARD_TYPE_MMC;
      DC('M');
      return SDCARD_RESULT_OK;
    }
    /* send SD_SEND_OP_COND */
    res = begin_command(SD_SEND_OP_COND, 1UL<<30);
    end_command();
    if(res == 0) {
      *is_sd = SDCARD_TYPE_SD;
      DC('S');
      return SDCARD_RESULT_OK;
    }
    if(timer_millis_timed_out(t0, SD_WRITE_TIMEOUT)) {
      return SDCARD_RESULT_FAILED_SENDOP;
    }
    /* keep watchdog happy */
    system_wdt_reset();
  }
}

u08 detect_sdhc(u08 *sd_type)
{
  u08 res = begin_command(CMD_READ_OCR, 0);
  if(res <= 1) {
    u32 arg = read_u32();
    DC('?'); DL(arg);
    if(arg & 0x40000000UL) {
      DC('H');
      *sd_type = SDCARD_TYPE_SDHC;
    } else {
      DC('S');
    }
  }
  end_command();
  return SDCARD_RESULT_OK;
}

static u08 send_op_cond(void)
{
  timer_ms_t t0 = timer_millis();
  while(1) {
    /* send CMD1_SEND_OP_COND */
    u08 res = begin_command(CMD1_SEND_OP_COND, 1UL<<30);
    end_command();
    if(res == 0) {
      return SDCARD_RESULT_OK;
    }
    if(timer_millis_timed_out(t0, SD_WRITE_TIMEOUT)) {
      return SDCARD_RESULT_FAILED_SENDOP;
    }
    /* keep watchdog happy */
    system_wdt_reset();
  }
}

static u08 set_crc_mode(u08 on)
{
  DS("CRC:"); DB(on);
  u08 res = begin_command(CMD_CRC_ON_OFF, on ? 1 : 0);
  end_command();
  return (res > 1) ? SDCARD_RESULT_FAILED_CRC_MODE : SDCARD_RESULT_OK;
}

static u08 set_blocksize(void)
{
  u08 res = begin_command(CMD_SET_BLOCKLEN, 512);
  end_command();
  return (res != 0) ? SDCARD_RESULT_FAILED_BLOCKLEN : SDCARD_RESULT_OK;
}

u08 sdcard_init(void)
{
  spi_set_speed(SPI_SPEED_SLOW);

  // 1. CMD0 enter idle state
  u08 result = enter_idle();
  if(result != SDCARD_RESULT_OK) {
    goto init_fail;
  }

  // 2. CMD8 (SEND IF COND)
  result = send_if_cond();
  if(result != SDCARD_RESULT_OK) {
    goto init_fail;
  }

  // 3. SD_SEND_OP_COND (init SD/HC)
  u08 type;
  result = sd_send_op_cond(&type);
  if(result != SDCARD_RESULT_OK) {
    goto init_fail;
  }

  // 4. detect SDHC
  if(type == SDCARD_TYPE_SD) {
    result = detect_sdhc(&type);
    if(result != SDCARD_RESULT_OK) {
      goto init_fail;
    }
  }

  // 5. SEND_OP_COND (init MMC) (ignored by SD/HC)
  result = send_op_cond();
  if(result != SDCARD_RESULT_OK) {
    goto init_fail;
  }

#ifdef USE_CRC
  const u08 use_crc = 1;
#else
  const u08 use_crc = 0;
#endif

  // 6. set CRC mode
  result = set_crc_mode(use_crc);
  if(result != SDCARD_RESULT_OK) {
    goto init_fail;
  }

  // 7. set blocklen
  result = set_blocksize();
  if(result != SDCARD_RESULT_OK) {
    goto init_fail;
  }

  // OK!
  result = SDCARD_RESULT_OK;
  card_type = type;
init_fail:
  spi_set_speed(SPI_SPEED_MAX);

  return result;
}

u08 sdcard_get_type(void)
{
  return card_type;
}


static uint32_t get_bits(void *buffer, uint16_t start, int8_t bits)
{
  uint8_t *buf = buffer;
  uint32_t result = 0;

  if ((start % 8) != 0) {
    /* Unaligned start */
    result += buf[start / 8] & (0xff >> (start % 8));
    bits  -= 8 - (start % 8);
    start += 8 - (start % 8);
  }
  while (bits >= 8) {
    result = (result << 8) + buf[start / 8];
    start += 8;
    bits -= 8;
  }
  if (bits > 0) {
    result = result << bits;
    result = result + (buf[start / 8] >> (8-bits));
  } else if (bits < 0) {
    /* Fraction of a single byte */
    result = result >> -bits;
  }
  return result;
}

static u08 wait_data_token(void)
{
  timer_ms_t t0 = timer_millis();
  DC('{');
  while(1) {
    u08 res = spi_in();
    DB(res); DC(',');
    if(res == 0xfe) {
      break;
    }
    /* check timeout */
    if(timer_millis_timed_out(t0, SD_TOKEN_TIMEOUT)) {
      return SDCARD_RESULT_FAILED_TOKEN;
    }
    /* keep watchdog happy */
    system_wdt_reset();
  }
  DC('}');
  return SDCARD_RESULT_OK;
}

static u08 wait_busy(void)
{
  timer_ms_t t0 = timer_millis();
  DC('<');
  while(1) {
    u08 res = spi_in();
    DB(res); DC(',');
    if(res == 0xff) {
      break;
    }
    /* check timeout */
    if(timer_millis_timed_out(t0, SD_BUSY_TIMEOUT)) {
      return SDCARD_RESULT_FAILED_TOKEN;
    }
    /* keep watchdog happy */
    system_wdt_reset();
  }
  DC('>');
  return SDCARD_RESULT_OK;
}

u08 sdcard_get_capacity(u32 *num_blocks)
{
  u08 res = begin_command(CMD_SEND_CSD, 0);
  if(res != 0) {
    end_command();
    return SDCARD_RESULT_FAILED_SEND_CSD;
  }

  res = wait_data_token();
  if(res != SDCARD_RESULT_OK) {
    end_command();
    return res;
  }

  // read result
  u08 buf[18];
  for(int i=0;i<18;i++) {
    buf[i] = spi_in();
  }
  end_command();

  u32 capacity;
  if (card_type == SDCARD_TYPE_SDHC) {
    /* Special CSD for SDHC cards */
    capacity = (1 + get_bits(buf,127-69,22)) * 1024;
  } else {
    /* Assume that MMC-CSD 1.0/1.1/1.2 and SD-CSD 1.1 are the same... */
    u08 exponent = 2 + get_bits(buf, 127-49, 3);
    capacity = 1 + get_bits(buf, 127-73, 12);
    exponent += get_bits(buf, 127-83,4) - 9;
    while (exponent--) capacity *= 2;
  }
  *num_blocks = capacity;
  return SDCARD_RESULT_OK;
}

u08 sdcard_read(u32 block_no, u08 *data)
{
  if (card_type != SDCARD_TYPE_SDHC) {
    // convert to byte offset
    block_no <<= 9;
  }

  u08 res = begin_command(CMD_READ_BLOCK, block_no);
  if(res != 0) {
    end_command();
    return SDCARD_RESULT_FAILED_READ;
  }

  res = wait_data_token();
  if(res != SDCARD_RESULT_OK) {
    end_command();
    return res;
  }

#ifdef USE_CRC
  u16 crc = 0;

  // read data
  for(u16 i=0;i<512;i++) {
    u08 d = spi_in();
    *data++ = d;
    crc = crc_xmodem_update(crc, d);
  }

  // read crc
  u08 crc_hi = spi_in();
  u08 crc_lo = spi_in();
  u16 got_crc = crc_hi << 8 | crc_lo;
  DC('C'); DW(crc); DC('='); DW(got_crc);

  if(got_crc != crc) {
    end_command();
    return SDCARD_RESULT_FAILED_CRC;
  }
#else
  // read data
  for(u16 i=0;i<512;i++) {
    *data++ = spi_in();
  }
#endif

  end_command();
  return SDCARD_RESULT_OK;
}

u08 sdcard_write(u32 block_no, const u08 *data)
{
  if (card_type != SDCARD_TYPE_SDHC) {
    // convert to byte offset
    block_no <<= 9;
  }

  u08 res = begin_command(CMD_WRITE_BLOCK, block_no);
  if(res != 0) {
    end_command();
    return SDCARD_RESULT_FAILED_WRITE;
  }

  // data token
  spi_out(0xfe);

#ifdef USE_CRC
  u16 crc = 0;

  // write data
  for(u16 i=0;i<512;i++) {
    u08 d = *data++;
    spi_out(d);
    crc = crc_xmodem_update(crc, d);
  }

  DC('C'); DW(crc);
  u08 crc_hi = (u08)(crc >> 8);
  u08 crc_lo = (u08)(crc & 0xff);

  // crc
  spi_out(crc_hi);
  spi_out(crc_lo);
#else
  // write data
  for(u16 i=0;i<512;i++) {
    spi_out(*data++);
  }

  // crc (dummy)
  spi_out(0xff);
  spi_out(0xff);
#endif

  // read status
  res = spi_in();
  DC('*'); DB(res);
  if((res & 0x1f) != 5) {
    // TBD: retry
    end_command();
    return SDCARD_RESULT_FAILED_WRITE;
  }

  res = wait_busy();
  end_command();

  return res;
}
