#ifndef SDCARD_H
#define SDCARD_H

#define SDCARD_RESULT_OK               0
#define SDCARD_RESULT_FAILED_IDLE      1
#define SDCARD_RESULT_FAILED_VOLTAGE   2
#define SDCARD_RESULT_FAILED_SENDOP    3
#define SDCARD_RESULT_FAILED_CRC_MODE  4
#define SDCARD_RESULT_FAILED_BLOCKLEN  5

#define SDCARD_TYPE_MMC         0
#define SDCARD_TYPE_SD          1
#define SDCARD_TYPE_SDHC        2

extern u08 sdcard_init(void);

#endif
