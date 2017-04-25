#ifndef HANDLER_H
#define HANDLER_H

#define HANDLER_OK          0

typedef void (*init_func_t)(void);
typedef void (*work_func_t)(void);
typedef u08 *(*read_msg_prepare_func_t)(u16 *size);
typedef void (*read_msg_done_func_t)(void);
typedef u08 *(*write_msg_prepare_func_t)(u16 *max_size);
typedef void (*write_msg_done_func_t)(u16 size);

struct handler {
  init_func_t               init_func;
  work_func_t               work_func;
  read_msg_prepare_func_t   read_msg_prepare;
  read_msg_done_func_t      read_msg_done;
  write_msg_prepare_func_t  write_msg_prepare;
  write_msg_done_func_t     write_msg_done;
};
typedef struct handler handler_t;

extern const u08 handler_table_size ROM_ATTR;
extern const handler_t handler_table[] ROM_ATTR;

#define HANDLER_TABLE_SIZE           sizeof(handler_table)/sizeof(handler_table[0])

#define HANDLER_TABLE_BEGIN          const handler_t handler_table[] ROM_ATTR = {
#define HANDLER_TABLE_END            }; const u08 handler_table_size = HANDLER_TABLE_SIZE;

#define HANDLER_ENTRY(i,w,rp,rd,wp,wd) { i,w,rp,rd,wp,wd }

extern void handler_init(void);
extern void handler_work(void);
extern u08 *handler_read_msg_prepare(u08 chn, u16 *size);
extern void handler_read_msg_done(u08 chn);
extern u08 *handler_write_msg_prepare(u08 chn, u16 *max_size);
extern void handler_wrtie_msg_done(u08 chn, u16 size);

#endif
