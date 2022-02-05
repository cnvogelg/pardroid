#ifndef BOOTBASE_H
#define BOOTBASE_H

#define BOOT_STATUS_OK                      0
#define BOOT_STATUS_INVALID_PAGE_SIZE       1

#define BOOTBASE_RET_RUN_APP                0
#define BOOTBASE_RET_CMD_LOOP               1

extern void bootbase_main(u16 page_size, u08 *page_buf);

extern void boot_wdt_reset(void);

#endif
