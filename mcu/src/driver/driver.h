#ifndef DRIVER_H
#define DRIVER_H

/* status messages */
#define DRIVER_OK               0
#define DRIVER_ERROR_INDEX      1
#define DRIVER_INIT_FAILED      2

/* flags */
#define DRIVER_FLAG_INIT        1

typedef void (*drv_reset_func_t)(u08 did);
typedef u08  (*drv_init_func_t)(u08 did);
typedef void (*drv_work_func_t)(u08 did, u08 flags);

typedef u08  (*drv_open_func_t)(u08 did);
typedef void (*drv_close_func_t)(u08 did);
typedef u16  (*drv_read_func_t)(u08 did, u08 *buf, u16 size);
typedef u16  (*drv_write_func_t)(u08 did, u08 *buf, u16 size);

struct driver {
  // funcs
  drv_reset_func_t              reset_func;
  drv_init_func_t               init_func;
  drv_work_func_t               work_func;
  drv_open_func_t               open_func;
  drv_close_func_t              close_func;
  drv_read_func_t               read_func;
  drv_write_func_t              write_func;
  // data
  u16                           mtu_max;
  u16                           mtu_min;
};
typedef struct driver driver_t;

struct driver_data {
  u08  flags;
};
typedef struct driver_data driver_data_t;

typedef const driver_t *driver_ptr_t;


extern const u08 driver_table_size ROM_ATTR;
extern const driver_ptr_t driver_table[] ROM_ATTR;
extern driver_data_t driver_data_table[];


#define DRIVER_GET_TABLE_SIZE()     read_rom_char(&driver_table_size)
#define DRIVER_TABLE_SIZE           sizeof(driver_table)/sizeof(driver_table[0])

#define DRIVER_TABLE_BEGIN          const driver_ptr_t driver_table[] ROM_ATTR = {
#define DRIVER_TABLE_END            }; \
                                    const u08 driver_table_size = DRIVER_TABLE_SIZE; \
                                    driver_data_t driver_data_table[DRIVER_TABLE_SIZE];
#define DRIVER_TABLE_ENTRY(name)    &drv_ ## name

#define DRIVER_DEFINE(name)         extern const driver_t drv_ ## name ROM_ATTR;
#define DRIVER_BEGIN(name)          const driver_t drv_ ## name ROM_ATTR = {
#define DRIVER_END                  };

#define DRIVER_RESET()              driver_reset(DRIVER_TABLE_SIZE)
#define DRIVER_INIT()               driver_init(DRIVER_TABLE_SIZE)
#define DRIVER_WORK()               driver_work(DRIVER_TABLE_SIZE)

#define DRIVER_GET_DATA(x)          &driver_data_table[x]

static inline driver_ptr_t driver_get_quick(u08 did)
{
  return (driver_ptr_t)read_rom_rom_ptr(&driver_table[did]);
}

static inline driver_ptr_t driver_get(u08 did)
{
  u08 max = read_rom_char(&driver_table_size);
  if(did < max) {
    return (driver_ptr_t)read_rom_rom_ptr(&driver_table[did]);
  } else {
    return 0;
  }
}

extern void driver_reset(u08 num);
extern void driver_init(u08 num);
extern void driver_work(u08 num);

extern u08  driver_open(u08 did);
extern void driver_close(u08 did);

#endif
