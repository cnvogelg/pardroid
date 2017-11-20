/*
 * sd card routines
 *
 * see:
 * - http://bikealive.nl/sd-v2-initialization.html
 * - https://yannik520.github.io/sdio.html
 */

#include "autoconf.h"
#include "types.h"
#include "timer.h"
#include "spi.h"
#include "system.h"

#define DEBUG CONFIG_DEBUG_DRIVER_SDCARD

#include "debug.h"

#include "sdcard.h"
#include "sdcard_defs.h"

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
#define SD_AUTO_RETRIES     10
#define SD_INIT_RETRIES     3

static uint8_t CRC7(const uint8_t* data, uint8_t n) {
  uint8_t crc = 0;
  for (uint8_t i = 0; i < n; i++) {
    uint8_t d = data[i];
    for (uint8_t j = 0; j < 8; j++) {
      crc <<= 1;
      if ((d & 0x80) ^ (crc & 0x80)) {
        crc ^= 0x09;
      }
      d <<= 1;
    }
  }
  return (crc << 1) | 1;
}

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
  buf[5] = CRC7(buf, 5);

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
  u08 is_sd;
  result = sd_send_op_cond(&is_sd);
  if(result != SDCARD_RESULT_OK) {
    goto init_fail;
  }

  // 4. detect SDHC
  result = detect_sdhc(&is_sd);
  if(result != SDCARD_RESULT_OK) {
    goto init_fail;
  }

  // 5. SEND_OP_COND (init MMC) (ignored by SD/HC)
  result = send_op_cond();
  if(result != SDCARD_RESULT_OK) {
    goto init_fail;
  }

  // 6. set CRC mode
  result = set_crc_mode(0);
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
init_fail:
  spi_set_speed(SPI_SPEED_MAX);

  return result;
}
