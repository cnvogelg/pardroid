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

u08 card_command(u08 cmd, u32 arg)
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

    DC('['); DB(cmd); DC('@'); DB(buf[5]);
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
      spi_disable_cs();
      errors++;
    } else {
      break;
    }
  }
  DC('='); DB(res); DC(']');
  return res;
}

u08 sdcard_init(void)
{
  u08 result = SDCARD_RESULT_OK;

  spi_set_speed(SPI_SPEED_SLOW);

  u08 tries = 3;
retry:
  // must supply min of 74 clock cycles with CS high.
  DC('a');
  spi_disable_cs();
  for (uint8_t i = 0; i < 10; i++) {
    spi_out(0XFF);
  }

  // command to go idle in SPI mode
  u08 res = card_command(CMD0, 0);
  spi_disable_cs();
  if(res & 0x80) {
    result = SDCARD_RESULT_FAILED_IDLE;
    goto init_fail;
  }
  if(res != STATUS_IDLE_STATE) {
    if (--tries) {
      goto retry;
    } else {
      return SDCARD_RESULT_FAILED_IDLE;
    }
  }

init_fail:
  spi_set_speed(SPI_SPEED_MAX);

  return result;
}
