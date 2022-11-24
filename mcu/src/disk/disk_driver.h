/* declare disk drivers used by disk service */

#ifndef DISK_DRIVER_H
#define DISK_DRIVER_H

#define DISK_DRIVER_INVALID_ID      0xff

/* ----- open/close/reset ----- */
typedef u08 (*disk_open_func_t)(u08 unit, u08 flags);
typedef void (*disk_close_func_t)(u08 unit);

/* ----- get info ----- */
typedef u08 (*disk_get_size_func_t)(u08 unit, disk_size_t *size);
typedef u08 (*disk_get_geo_func_t)(u08 unit, disk_geo_t *geo);

/* ----- read/write ----- */
typedef u08 (*disk_read_func_t)(u08 unit, u32 lba, u16 num_blocks, u08 *data);
typedef u08 (*disk_write_func_t)(u08 unit, u32 lba, u16 num_blocks, u08 *data);

/* ----- disk_driver's configuration ----- */
struct disk_driver_config {
  // type of disk driver
  u08                           device;
  // number of units the driver supports
  u08                           num_units;
  // name of the driver
  rom_pchar                     name;
};
typedef struct disk_driver_config disk_driver_config_t;

/* RAM data of driver */
struct disk_driver_data {
  u08    unit_mask;
};
typedef struct disk_driver_data disk_driver_data_t;

/* ----- the disk_driver ----- */
struct disk_driver {
  // internal pointer to driver data
  disk_driver_data_t        *data;

  // constant config
  disk_driver_config_t       config;

  // ----- function table of driver -----
  disk_open_func_t               open;
  disk_close_func_t              close;

  // ----- info -----
  disk_get_size_func_t           get_size;
  disk_get_geo_func_t            get_geo;

  /* ----- read/write ----- */
  disk_read_func_t               read;;
  disk_write_func_t              write;
};
typedef struct disk_driver disk_driver_t;
typedef const disk_driver_t *disk_driver_ptr_t;

/* ----- macros to help create disk driver ----- */

#define DISK_DRIVER_DECLARE(name) \
  extern const disk_driver_t name ROM_ATTR;

#define DISK_DRIVER_BEGIN(name)           \
                                          disk_driver_data_t name ## _data; \
                                          const disk_driver_t name ROM_ATTR = { \
                                            .data = &name ## _data,
#define DISK_DRIVER_END                   };

#define DISK_DRIVER_GET_DEVICE(disk)       read_rom_char(&disk->config.device)
#define DISK_DRIVER_GET_NUM_UNITS(disk)    read_rom_char(&disk->config.num_units)
#define DISK_DRIVER_GET_NAME(disk)         read_rom_rom_ptr(&disk->config.name)

#define DISK_DRIVER_GET_DATA(hnd)          (disk_driver_data_t *)read_rom_ram_ptr(&hnd->data)

#define DISK_DRIVER_FUNC_OPEN(disk)        ((disk_open_func_t)read_rom_rom_ptr(&disk->open))
#define DISK_DRIVER_FUNC_CLOSE(disk)       ((disk_close_func_t)read_rom_rom_ptr(&disk->close))

#define DISK_DRIVER_FUNC_GET_SIZE(disk)    ((disk_get_size_func_t)read_rom_rom_ptr(&disk->get_size))
#define DISK_DRIVER_FUNC_GET_GEO(disk)     ((disk_get_geo_func_t)read_rom_rom_ptr(&disk->get_geo))

#define DISK_DRIVER_FUNC_READ(disk)        ((disk_read_func_t)read_rom_rom_ptr(&disk->read))
#define DISK_DRIVER_FUNC_WRITE(disk)       ((disk_write_func_t)read_rom_rom_ptr(&disk->write))

/* ----- macros to create the disk_driver table ----- */

#define DISK_DRIVER_TABLE_DECLARE \
  extern const disk_driver_ptr_t disk_driver_table[] ROM_ATTR; \
  extern const u08 disk_driver_table_size ROM_ATTR;

#define DISK_DRIVER_TABLE_BEGIN   const disk_driver_ptr_t disk_driver_table[] ROM_ATTR = {

#define DISK_DRIVER_TABLE_END     }; \
  const u08 disk_driver_table_size ROM_ATTR = sizeof(disk_driver_table) / sizeof(disk_driver_ptr_t);

#define DISK_DRIVER_TABLE_GET_SIZE()   read_rom_char(&disk_driver_table_size)

#define DISK_DRIVER_TABLE_GET_ENTRY(x) (disk_driver_ptr_t)read_rom_rom_ptr(&disk_driver_table[x]);

#endif
