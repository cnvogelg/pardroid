#ifndef HANDLER_H
#define HANDLER_H

#define HANDLER_PROP_NONE               0x00
#define HANDLER_PROP_WORK_CLOSED        0x01

/* the channel handler provides a set of functions
   operating on the channel */

typedef u08  (*hnd_open_func_t)(u08 chn);
typedef void (*hnd_work_func_t)(u08 chn);
typedef void (*hnd_close_func_t)(u08 chn);
typedef void (*hnd_reset_func_t)(u08 chn);

typedef u16  (*hnd_read_begin_func_t)(u08 chan, u16 mtu, u32 offset);
typedef u08 *(*hnd_read_chunk_begin_func_t)(u08 chan, u16 chunk_offset, u16 chunk_size);
typedef void (*hnd_read_chunk_end_func_t)(u08 chan);
typedef void (*hnd_read_end_func_t)(u08 chan, u08 cancelled);

typedef void (*hnd_write_begin_func_t)(u08 chan, u16 mtu, u32 offset, u16 size);
typedef u08 *(*hnd_write_chunk_begin_func_t)(u08 chan, u16 chunk_offset, u16 chunk_size);
typedef void (*hnd_write_chunk_end_func_t)(u08 chan);
typedef void (*hnd_write_end_func_t)(u08 chan, u08 cancelled);

typedef u16  (*hnd_set_mtu_func_t)(u08 chan, u16 new_mtu);

struct handler {
  // constant parameters of the handler
  u16                           def_mtu;
  u16                           max_size;
  u16                           mode;

  hnd_open_func_t               open;
  hnd_work_func_t               work;
  hnd_close_func_t              close;
  hnd_reset_func_t              reset;

  hnd_read_begin_func_t         read_begin;
  hnd_read_chunk_begin_func_t   read_chunk_begin;
  hnd_read_chunk_end_func_t     read_chunk_end;
  hnd_read_end_func_t           read_end;

  hnd_write_begin_func_t        write_begin;
  hnd_write_chunk_begin_func_t  write_chunk_begin;
  hnd_write_chunk_end_func_t    write_chunk_end;
  hnd_write_end_func_t          write_end;

  hnd_set_mtu_func_t            set_mtu;
};
typedef struct handler handler_t;
typedef const handler_t *handler_ptr_t;

#define HANDLER_DEFINE(name)         extern const handler_t name ROM_ATTR;
#define HANDLER_BEGIN(name)          const handler_t name ROM_ATTR = {
#define HANDLER_END                  };

#define HANDLER_GET_DEF_MTU(hnd)     read_rom_word(&hnd->def_mtu)
#define HANDLER_GET_MAX_SIZE(hnd)    read_rom_word(&hnd->max_size)
#define HANDLER_GET_MODE(hnd)        read_rom_word(&hnd->mode)

#define HANDLER_FUNC_OPEN(hnd)        ((hnd_open_func_t)read_rom_rom_ptr(&hnd->open))
#define HANDLER_FUNC_WORK(hnd)        ((hnd_work_func_t)read_rom_rom_ptr(&hnd->work))
#define HANDLER_FUNC_CLOSE(hnd)       ((hnd_close_func_t)read_rom_rom_ptr(&hnd->close))
#define HANDLER_FUNC_RESET(hnd)       ((hnd_reset_func_t)read_rom_rom_ptr(&hnd->reset))

#define HANDLER_FUNC_SET_MTU(hnd)     ((hnd_set_mtu_func_t)read_rom_rom_ptr(&hnd->set_mtu))

#define HANDLER_FUNC_READ_BEGIN(hnd)  ((hnd_read_begin_func_t)read_rom_rom_ptr(&hnd->read_begin))
#define HANDLER_FUNC_READ_CHUNK_BEGIN(hnd)  ((hnd_read_chunk_begin_func_t)read_rom_rom_ptr(&hnd->read_chunk_begin))
#define HANDLER_FUNC_READ_CHUNK_END(hnd)    ((hnd_read_chunk_end_func_t)read_rom_rom_ptr(&hnd->read_chunk_end))
#define HANDLER_FUNC_READ_END(hnd)    ((hnd_read_end_func_t)read_rom_rom_ptr(&hnd->read_end))

#define HANDLER_FUNC_WRITE_BEGIN(hnd) ((hnd_write_begin_func_t)read_rom_rom_ptr(&hnd->write_begin))
#define HANDLER_FUNC_WRITE_CHUNK_BEGIN(hnd) ((hnd_write_chunk_begin_func_t)read_rom_rom_ptr(&hnd->write_chunk_begin))
#define HANDLER_FUNC_WRITE_CHUNK_END(hnd)   ((hnd_write_chunk_end_func_t)read_rom_rom_ptr(&hnd->write_chunk_end))
#define HANDLER_FUNC_WRITE_END(hnd)   ((hnd_write_end_func_t)read_rom_rom_ptr(&hnd->write_end))

#endif
