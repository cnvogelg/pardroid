#ifndef DRIVER_H
#define DRIVER_H

/* status messages */
#define DRIVER_OK                   0
#define DRIVER_ERROR_INIT_FAILED    1
#define DRIVER_ERROR_WRONG_SLOT     2
#define DRIVER_ERROR_NO_DRIVER      3
#define DRIVER_ERROR_ALREADY_OPEN   4
#define DRIVER_ERROR_NOT_OPEN       5
#define DRIVER_ERROR_READ           6
#define DRIVER_ERROR_WRITE          7

#define DRIVER_MAX_SLOTS        7

/* flags */
#define DRIVER_NO_INIT          0x80

/* type */
#define DRIVER_TYPE_BLK         0
#define DRIVER_TYPE_ETH         1
#define DRIVER_TYPE_UDP         2

// dynamic data of driver
struct driver_data {
  u08  open_slots;
};

struct driver;
typedef struct driver driver_t;
typedef struct driver_data driver_data_t;
typedef const driver_t *driver_ptr_t;

typedef void (*drv_reset_func_t)(u08 did);
typedef u08  (*drv_init_func_t)(u08 did);
typedef void (*drv_work_func_t)(u08 did, struct driver_data *data);

typedef u08  (*drv_open_func_t)(u08 did, u08 slot);
typedef void (*drv_close_func_t)(u08 did, u08 slot);

// block API
typedef u08  (*drv_blk_read_func_t)(u08 did, u08 slot, u32 blk_no, u08 *buf);
typedef u08  (*drv_blk_write_func_t)(u08 did, u08 slot, u32 blk_no, const u08 *buf);
typedef u08  (*drv_blk_get_capacity_func_t)(u08 did, u08 slot, u32 *num_blocks);

// packet API
typedef u08  (*drv_pkt_rx_pending_func_t)(u08 did, u08 slot, u08 *num_pending);

// static data of driver
struct driver {
  // --- funcs ---
  drv_reset_func_t              reset_func;
  drv_init_func_t               init_func;
  drv_work_func_t               work_func;
  drv_open_func_t               open_func;
  drv_close_func_t              close_func;

  // blk
  drv_blk_read_func_t           blk_read_func;
  drv_blk_write_func_t          blk_write_func;
  drv_blk_get_capacity_func_t   blk_get_capacity_func;

  // pkt
  drv_pkt_rx_pending_func_t     pkt_rx_pending_func;

  // --- consts ---
  u08                           type;
  u08                           num_slots;
  // pkt consts
  u16                           mtu_max;
  u16                           mtu_min;
  // blk consts
  u16                           blk_size;
};

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

#define DRIVER_GET_DATA(x)          &driver_data_table[x]

// function factories
#define DRIVER_GET_CONST_BYTE(name)  \
INLINE u08 driver_get_ ## name (u08 did) { \
  driver_ptr_t drv = driver_get(did); \
  if(drv == 0) { \
    return 0; \
  } \
  return read_rom_char(&drv-> name); \
}

#define DRIVER_GET_CONST_WORD(name)  \
INLINE u16 driver_get_ ## name (u08 did) { \
  driver_ptr_t drv = driver_get(did); \
  if(drv == 0) { \
    return 0; \
  } \
  return read_rom_word(&drv-> name); \
}

#define DRIVER_CALL_BEGIN(name, args...) \
INLINE u08 driver_ ## name (u08 did, u08 slot, args) { \
  driver_ptr_t drv = driver_get(did); \
  if(drv == 0) { \
    return DRIVER_ERROR_NO_DRIVER; \
  } \
  u08 num_slots = read_rom_char(&drv->num_slots); \
  if(slot >= num_slots) { \
    return DRIVER_ERROR_WRONG_SLOT; \
  } \
  drv_ ## name ## _func_t func = (drv_ ## name ## _func_t)read_rom_rom_ptr(&drv-> name ## _func); \
  if(func != 0) {

#define DRIVER_CALL_END(vals...) \
    func(did, slot, vals); \
  } else { \
    return DRIVER_OK; \
  } \
}

// helper
INLINE driver_ptr_t driver_get_quick(u08 did)
{
  return (driver_ptr_t)read_rom_rom_ptr(&driver_table[did]);
}

INLINE driver_ptr_t driver_get(u08 did)
{
  u08 max = read_rom_char(&driver_table_size);
  if(did < max) {
    return (driver_ptr_t)read_rom_rom_ptr(&driver_table[did]);
  } else {
    return 0;
  }
}

// driver const getters
DRIVER_GET_CONST_BYTE(type)
DRIVER_GET_CONST_BYTE(num_slots)
DRIVER_GET_CONST_WORD(mtu_max)
DRIVER_GET_CONST_WORD(mtu_min)
DRIVER_GET_CONST_WORD(blk_size)

// API
extern void driver_reset(u08 num);
extern void driver_init(u08 num);
extern void driver_work(u08 num);

extern u08  driver_open(u08 did, u08 slot);
extern u08  driver_close(u08 did, u08 slot);

// blk API
DRIVER_CALL_BEGIN(blk_read, u32 blk_no, u08 *buf);
DRIVER_CALL_END(blk_no, buf)

DRIVER_CALL_BEGIN(blk_write, u32 blk_no, const u08 *buf);
DRIVER_CALL_END(blk_no, buf)

DRIVER_CALL_BEGIN(blk_get_capacity, u32 *num_blocks);
DRIVER_CALL_END(num_blocks)

// pkt API
DRIVER_CALL_BEGIN(pkt_rx_pending, u08 *num_pending)
DRIVER_CALL_END(num_pending)

#endif
