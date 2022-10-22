#ifndef SDCARD_H
#define SDCARD_H

#define SDCARD_RESULT_OK               0
#define SDCARD_RESULT_FAILED_IDLE      1
#define SDCARD_RESULT_FAILED_VOLTAGE   2
#define SDCARD_RESULT_FAILED_SENDOP    3
#define SDCARD_RESULT_FAILED_CRC_MODE  4
#define SDCARD_RESULT_FAILED_BLOCKLEN  5
#define SDCARD_RESULT_FAILED_TOKEN     6
#define SDCARD_RESULT_FAILED_SEND_CSD  7
#define SDCARD_RESULT_FAILED_READ      8
#define SDCARD_RESULT_FAILED_WRITE     9
#define SDCARD_RESULT_FAILED_BUSY      10
#define SDCARD_RESULT_FAILED_CRC       11
#define SDCARD_RESULT_FAILED_SPI_IN_USE 12
#define SDCARD_RESULT_FAILED_NO_INIT   13

#define SDCARD_TYPE_NONE        0
#define SDCARD_TYPE_MMC         1
#define SDCARD_TYPE_SD          2
#define SDCARD_TYPE_SDHC        3

extern u08 sdcard_init(u08 spi_cs);
extern u08 sdcard_acquire(u08 spi_cs);
extern void sdcard_release(void);

extern u08 sdcard_get_status(void);
extern u08 sdcard_get_type(void);
extern u08 sdcard_get_capacity(u32 *num_blocks);

extern u08 sdcard_read(u32 block_no, u08 *data);
extern u08 sdcard_write(u32 block_no, const u08 *data);
extern u08 sdcard_read_multi(u32 block_no, u08 *data, u32 num);
extern u08 sdcard_write_multi(u32 block_no, const u08 *data, u32 num);

#endif
