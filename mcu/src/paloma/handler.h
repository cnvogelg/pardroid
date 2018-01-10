#ifndef HANDLER_H
#define HANDLER_H

/* status codes */
#define HANDLER_OK              0
#define HANDLER_ERROR_INDEX     1
#define HANDLER_ALREADY_OPEN    2
#define HANDLER_CLOSED          3
#define HANDLER_NO_MEMORY       4
#define HANDLER_NO_FUNC         5
#define HANDLER_NO_CHANNEL      6

/* flags */
#define HANDLER_FLAG_NONE          0
#define HANDLER_FLAG_INIT          1
#define HANDLER_FLAG_OPEN          2
#define HANDLER_FLAG_ERROR         4

typedef u08  (*hnd_init_func_t)(u08 chn);
typedef void (*hnd_work_func_t)(u08 chn, u08 flags);
typedef u08  (*hnd_open_func_t)(u08 chn);
typedef void (*hnd_close_func_t)(u08 chn);
typedef u08  (*hnd_read_func_t)(u08 chn, u16 *size, u08 *buf);
typedef u08  (*hnd_write_func_t)(u08 chn, u16 size, u08 *buf);

struct handler {
  hnd_init_func_t               init_func;
  hnd_work_func_t               work_func;
  hnd_open_func_t               open_func;
  hnd_close_func_t              close_func;
  hnd_read_func_t               read_func;
  hnd_write_func_t              write_func;

  u16                           mtu_max;
  u16                           mtu_min;
};
typedef struct handler handler_t;

struct handler_data {
  u08  flags;
  u08  status;
  u08  channel;
  u16  mtu;
};
typedef struct handler_data handler_data_t;

typedef const handler_t *handler_ptr_t;


extern const u08 handler_table_size ROM_ATTR;
extern const handler_ptr_t handler_table[] ROM_ATTR;
extern handler_data_t handler_data_table[];


#define HANDLER_GET_TABLE_SIZE()     read_rom_char(&handler_table_size)
#define HANDLER_TABLE_SIZE           sizeof(handler_table)/sizeof(handler_table[0])

#define HANDLER_TABLE_BEGIN          const handler_ptr_t handler_table[] ROM_ATTR = {
#define HANDLER_TABLE_END            }; \
                                     const u08 handler_table_size = HANDLER_TABLE_SIZE; \
                                     handler_data_t handler_data_table[HANDLER_TABLE_SIZE];
#define HANDLER_TABLE_ENTRY(name)    &hnd_ ## name

#define HANDLER_DEFINE(name)         extern const handler_t hnd_ ## name ROM_ATTR;
#define HANDLER_BEGIN(name)          const handler_t hnd_ ## name ROM_ATTR = {
#define HANDLER_END                  };

#define HANDLER_GET_DATA(x)          &handler_data_table[x]


extern void handler_init(u08 num);
extern void handler_work(u08 num);

extern u08  handler_open(u08 chn);
extern u08  handler_close(u08 chn);
extern u08  handler_read(u08 chn, u16 *size, u08 *buf);
extern u08  handler_write(u08 chn, u16 size, u08 *buf);

extern void handler_set_status(u08 chn, u08 status);
extern void handler_get_mtu(u08 chn, u16 *mtu_max, u16 *mtu_min);

#endif
