#ifndef HANDLER_H
#define HANDLER_H

#define HANDLER_OK              0
#define HANDLER_ERROR_INDEX     1

typedef u08  (*init_func_t)(u08 chn);
typedef void (*work_func_t)(u08 chn, u08 flags);
typedef u08  (*open_func_t)(u08 chn);
typedef void (*close_func_t)(u08 chn);
typedef u08 *(*read_msg_prepare_func_t)(u08 chn, u16 *size);
typedef void (*read_msg_done_func_t)(u08 chn);
typedef u08 *(*write_msg_prepare_func_t)(u08 chn, u16 *max_size);
typedef void (*write_msg_done_func_t)(u08 chn, u16 size);

struct handler {
  init_func_t               init_func;
  work_func_t               work_func;
  open_func_t               open_func;
  close_func_t              close_func;
  read_msg_prepare_func_t   read_msg_prepare;
  read_msg_done_func_t      read_msg_done;
  write_msg_prepare_func_t  write_msg_prepare;
  write_msg_done_func_t     write_msg_done;
  u16                       mtu_max;
  u16                       mtu_min;
};
typedef struct handler handler_t;

typedef const handler_t *handler_ptr_t;

extern const u08 handler_table_size ROM_ATTR;
extern const handler_ptr_t handler_table[] ROM_ATTR;

#define HANDLER_GET_TABLE_SIZE()     read_rom_char(&handler_table_size)
#define HANDLER_TABLE_SIZE           sizeof(handler_table)/sizeof(handler_table[0])

#define HANDLER_TABLE_BEGIN          const handler_ptr_t handler_table[] ROM_ATTR = {
#define HANDLER_TABLE_END            }; const u08 handler_table_size = HANDLER_TABLE_SIZE;
#define HANDLER_TABLE_ENTRY(name)    &name

#define HANDLER_DEFINE(name)  extern const handler_t name ROM_ATTR;
#define HANDLER_BEGIN(name)   const handler_t name ROM_ATTR = {
#define HANDLER_END           };

extern u08  handler_init(u08 chn);
extern void handler_work(u08 chn, u08 flags);
extern u08  handler_open(u08 chn);
extern void handler_close(u08 chn);
extern u08 *handler_read_msg_prepare(u08 chn, u16 *size);
extern void handler_read_msg_done(u08 chn);
extern u08 *handler_write_msg_prepare(u08 chn, u16 *max_size);
extern void handler_wrtie_msg_done(u08 chn, u16 size);

extern void handler_get_mtu(u08 chn, u16 *mtu_max, u16 *mtu_min);

#endif
