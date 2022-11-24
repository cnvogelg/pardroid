/* access disks */

#ifndef DISK_H
#define DISK_H

#include "disk_dev.h"

/* max units allowed per device */
#define DISK_MAX_UNITS                    8

/* --- disk status --- */
#define DISK_OK                           0
#define DISK_ERROR_NO_DEVICE              1
#define DISK_ERROR_UNKNOWN_DEVICE         2
#define DISK_ERROR_NOT_OPEN               3
#define DISK_ERROR_ALREADY_OPEN           4
#define DISK_ERROR_INVALID_UNIT           5
#define DISK_ERROR_INVALID_SLOT           6
#define DISK_ERROR_UNIT_BUSY              7
#define DISK_ERROR_UNKNOWN_GEO            8
#define DISK_ERROR_NO_MEDIA               9
#define DISK_ERROR_NOT_WRITABLE           10
#define DISK_ERROR_READ                   11
#define DISK_ERROR_WRITE                  12

/* --- disk flags --- */
#define DISK_FLAGS_NONE                   0
#define DISK_FLAGS_READ_ONLY              1

/* --- types --- */
/* forward decl */
struct disk_driver;

/* disk instance data */
struct disk_handle {
  u08   device;
  u08   unit;
  u08   flags;
  const struct disk_driver *driver;
};
typedef struct disk_handle disk_handle_t;

/* disk size */
struct disk_size {
  u16  block_size;
  u32  total_blocks;
};
typedef struct disk_size disk_size_t;

/* disk geometry */
struct disk_geo {
  u16  cylinders;
  u16  heads;
  u16  sectors;
};
typedef struct disk_geo disk_geo_t;

/* --- API --- */
/* open disk device and unit */
extern u08 disk_open(disk_handle_t *hnd, u08 device, u08 unit, u08 flags);
/* close device */
extern u08 disk_close(disk_handle_t *hnd);

/* query size of open disk */
extern u08 disk_get_size(disk_handle_t *hnd, disk_size_t *size);
/* query geo of open disk */
extern u08 disk_get_geo(disk_handle_t *hnd, disk_geo_t *geo);

/* read disk block(s) */
extern u08 disk_read(disk_handle_t *hnd, u32 lba, u16 num_blocks, u08 *data);
/* write disk block(s) */
extern u08 disk_write(disk_handle_t *hnd, u32 lba, u16 num_blocks, u08 *data);

#endif
