#include "autoconf.h"
#include "types.h"

#define DEBUG CONFIG_DEBUG_DRIVER_FATFS

#include "debug.h"

#include "sdcard.h"

#include "ff.h"
#include "diskio.h" // from FatFS

/* ----- adapter for FatFS ----- */

DSTATUS disk_status(BYTE pdrv)
{
  u08 res = sdcard_get_status();
  DS("sd:dstatus="); DB(res); DNL;
  if(res == SDCARD_RESULT_OK) {
    return 0;
  }
  return STA_NOINIT;
}

DSTATUS disk_initialize(BYTE pdrv)
{
  // init has to be done before mount...
  u08 res = sdcard_get_status();
  DS("sd:dinit="); DB(res); DNL;
  if(res == SDCARD_RESULT_OK) {
    return 0;
  }
  return STA_NOINIT;;
}

DRESULT disk_read(BYTE pdrv, BYTE *buf, LBA_t sector, UINT count)
{
  u08 res;
  if(count == 1) {
    res = sdcard_read(sector, buf);
  } else {
    res = sdcard_read_multi(sector, buf, count);
  }
  DS("sd:dread@"); DL(sector); DC('+'); DL(count); DC('='); DB(res); DNL;
  if(res != SDCARD_RESULT_OK) {
    return RES_ERROR;
  }
  return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buf, LBA_t sector, UINT count)
{
  u08 res;
  if(count == 1) {
    res = sdcard_write(sector, buf);
  } else {
    res = sdcard_write_multi(sector, buf, count);
  }
  DS("sd:dwrite@"); DL(sector); DC('+'); DL(count); DC('='); DB(res); DNL;
  if(res != SDCARD_RESULT_OK) {
    return RES_ERROR;
  }
  return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buf)
{
  DS("sd:dioctl:"); DB(cmd); DNL;

  return RES_OK;
}

DWORD get_fattime (void)
{
  DS("sd:fattime"); DNL;

  return 0;
}
