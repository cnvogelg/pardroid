#ifndef PAMELA_HANDLER_H
#define PAMELA_HANDLER_H

/* if at least one channel is open to the handler
   it will be called regularly in the main loop
   with its work function.
   Do not block there for too long, otherwise
   the watchdog will trigger!

   the channel_mask gives the active connections to
   this handler.
*/
typedef void (*hnd_work_func_t)(u16 channel_mask);

/* a channel was opened to the given port */
typedef u08  (*hnd_open_func_t)(u08 chn, u16 port);
/* close the channel */
typedef void (*hnd_close_func_t)(u08 chn);
/* reset the channel.
   this keeps the channel open but all
   pending reads/writes are cancelled
   and any error flags are reset. */
typedef void (*hnd_reset_func_t)(u08 chn);

/* a new mtu was set by the host.

   return accepted value
*/
typedef u16 (*hnd_set_mtu_func_t)(u08 chn, u16 mtu);

/* ----- seek/tell ----- */
/* seek to a given position */
typedef void (*hnd_seek_func_t)(u08 chn, u32 pos);
/* return current position */
typedef u32 (*hnd_tell_func_t)(u08 chn);

/* ----- read ----- */
/* a read request arrived from the host with the
   desired max size to read.
   the handler will process the read operation and
   answer with pamela_read_ready()

   return PAMELA_OK or PAMELA_ERROR
*/
typedef u08 (*hnd_read_request_func_t)(u08 chan, u16 size);
/* a read operation that was reported ready is now
   finally retrieved from the host and this
   callback notifies the handler that the associated
   buffer/SPI is free again.
*/
typedef void (*hnd_read_done_func_t)(u08 chan, u08 *buf, u16 size);

/* ----- write ----- */
/* a write request arrived from the host with the
   desired size to read.
   the handler will process the read operation and
   answer with pamela_write_ready()

   return PAMELA_OK or PAMELA_ERROR
*/
typedef u08 (*hnd_write_request_func_t)(u08 chan, u16 size);
/* a write operation was reported ready is now
   finally retrieved from the host and this
   callback notifies the handler that the associated
   buffer/SPI is free again.
*/
typedef void (*hnd_write_done_func_t)(u08 chan, u08 *buf, u16 size);

/* ----- handler's configuration ----- */
struct pamela_handler_config {
  // the port range the handler will listen on
  u16                           port_begin;
  u16                           port_end;
  // the default MTU the handler will use
  u16                           def_mtu;
  // the maximum MTU the handler can use
  u16                           max_mtu;
};
typedef struct pamela_handler_config pamela_handler_config_t;

/* ----- the handler ----- */
struct pamela_handler {
  // constant config
  pamela_handler_config_t       config;

  // function table
  hnd_work_func_t               work;

  hnd_open_func_t               open;
  hnd_close_func_t              close;
  hnd_reset_func_t              reset;

  hnd_seek_func_t               seek;
  hnd_tell_func_t               tell;

  hnd_read_request_func_t       read_request;
  hnd_read_done_func_t          read_done;

  hnd_write_request_func_t      write_request;
  hnd_write_done_func_t         write_done;

  hnd_set_mtu_func_t            set_mtu;
};
typedef struct pamela_handler pamela_handler_t;
typedef const pamela_handler_t *pamela_handler_ptr_t;

/* ----- macros to help create handlers ----- */

#define HANDLER_DEFINE(name)         extern const pamela_handler_t name ROM_ATTR;
#define HANDLER_BEGIN(name)          const pamela_handler_t name ROM_ATTR = {
#define HANDLER_END                  };

#define HANDLER_GET_PORT_BEGIN(hnd)  read_rom_word(&hnd->config.port_begin)
#define HANDLER_GET_PORT_END(hnd)    read_rom_word(&hnd->config.port_end)
#define HANDLER_GET_DEF_MTU(hnd)     read_rom_word(&hnd->config.def_mtu)
#define HANDLER_GET_MAX_MTU(hnd)     read_rom_word(&hnd->config.max_mtu)

#define HANDLER_FUNC_WORK(hnd)        ((hnd_work_func_t)read_rom_rom_ptr(&hnd->work))

#define HANDLER_FUNC_OPEN(hnd)        ((hnd_open_func_t)read_rom_rom_ptr(&hnd->open))
#define HANDLER_FUNC_CLOSE(hnd)       ((hnd_close_func_t)read_rom_rom_ptr(&hnd->close))
#define HANDLER_FUNC_RESET(hnd)       ((hnd_reset_func_t)read_rom_rom_ptr(&hnd->reset))

#define HANDLER_FUNC_SEEK(hnd)        ((hnd_seek_func_t)read_rom_rom_ptr(&hnd->seek))
#define HANDLER_FUNC_TELL(hnd)        ((hnd_tell_func_t)read_rom_rom_ptr(&hnd->tell))

#define HANDLER_FUNC_READ_REQUEST(hnd ) ((hnd_read_request_func_t)read_rom_rom_ptr(&hnd->read_request))
#define HANDLER_FUNC_READ_DONE(hnd)     ((hnd_read_done_func_t)read_rom_rom_ptr(&hnd->read_done))

#define HANDLER_FUNC_WRITE_REQUEST(hnd) ((hnd_write_request_func_t)read_rom_rom_ptr(&hnd->write_request))
#define HANDLER_FUNC_WRITE_DONE(hnd)    ((hnd_write_done_func_t)read_rom_rom_ptr(&hnd->write_done))

#define HANDLER_FUNC_SET_MTU(hnd)       ((hnd_set_mtu_func_t)read_rom_rom_ptr(&hnd->set_mtu))

#endif
