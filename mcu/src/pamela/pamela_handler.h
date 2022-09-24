#ifndef PAMELA_HANDLER_H
#define PAMELA_HANDLER_H

/* ----- open/close/reset ----- */
typedef u08 (*hnd_open_func_t)(u08 chn, u16 port);
typedef u08 (*hnd_close_func_t)(u08 chn);
typedef u08 (*hnd_reset_func_t)(u08 chn);

/* ----- reconfig ----- */
typedef u16 (*hnd_set_mtu_func_t)(u08 chn, u16 mtu);

/* ----- seek/tell ----- */
typedef void (*hnd_seek_func_t)(u08 chn, u32 pos);
typedef u32 (*hnd_tell_func_t)(u08 chn);

/* ----- read ----- */
typedef u08 (*hnd_read_request_func_t)(u08 chn, u08 **buf, u16 *size);
typedef void (*hnd_read_done_func_t)(u08 chn, u08 *buf, u16 size);

/* ----- write ----- */
typedef u08 (*hnd_write_request_func_t)(u08 chn, u08 **buf, u16 *size);
typedef void (*hnd_write_done_func_t)(u08 chn, u08 *buf, u16 size);

/* ----- worker ----- */
typedef u08 (*hnd_work_func_t)(u08 chn);

/* ----- handler's configuration ----- */
struct pamela_handler_config {
  // the port range the handler will listen on
  u16                           port_begin;
  u16                           port_end;
  // the default MTU the handler will use
  u16                           def_mtu;
  // the maximum MTU the handler can use
  u16                           max_mtu;
  // the number of slots the handler can cope with
  u08                           max_slots;
};
typedef struct pamela_handler_config pamela_handler_config_t;

/* ----- the handler ----- */
struct pamela_handler {
  // constant config
  pamela_handler_config_t       config;

  // ----- function table of handler -----
  /* a channel was opened to the given port

     either process the open immediately and
     return either PAMELA_OK or an error.

     if PAMELA_BUSY is returned then the
     open operation takes some time and
     is continued with open_work_func.
  */
  hnd_open_func_t               open;
  /* continue opening port
     return PAMELA_OK or error to finish the open
     operation. use PAMELA_BUSY to continue
     getting calls.

     only required if open returns PAMELA_BUSY
  */
  hnd_open_func_t               open_work;

  /* close the channel. similar to open
     return PAMELA_OK or PAMELA_BUSY
  */
  hnd_close_func_t              close;
  /* close cont.
     return PAMELA_OK or PAMELA_BUSY
  */
  hnd_close_func_t              close_work;

  /* reset the channel.
     this keeps the channel open but all
     pending reads/writes are cancelled
     and any error flags are reset.
     return PAMELA_OK or PAMELA_BUSY
  */
  hnd_reset_func_t              reset;
  /* cont. reset
     return PAMELA_OK or PAMELA_BUSY
  */
  hnd_reset_func_t              reset_work;

  /* seek to a given position */
  hnd_seek_func_t               seek;
  /* return current seek position */
  hnd_tell_func_t               tell;

  /* ----- read ----- */
  /* a read request arrived from the host with the
     desired max size to read (*size).

     either directly process the request and return
     the final size in (*size) and the buffer with
     data in *buf and return PAMELA_OK.
     or return a read ERROR

     otherwise return PAMELA_BUSY and get
     additional calls in read_work()
  */
  hnd_read_request_func_t       read_request;
  /* continue read operation
     return PAMELA_OK when done, an error or BUSY
     to continue
  */
  hnd_read_request_func_t       read_work;
  /* a read operation that was reported ready is now
     finally retrieved from the host and this
     callback notifies the handler that the associated
     buffer/SPI is free again.
  */
  hnd_read_done_func_t          read_done;

  /* ----- write ----- */
  /* a write request arrived from the host with the
     desired size to read.
     the handler will process the read operation and
     answer with pamela_write_ready()

     return PAMELA_OK or error or PAMELA_BUSY
  */
  hnd_write_request_func_t      write_request;
  /* continue WRITE operation
     return PAMELA_OK when done, an error or BUSY
     to continue
  */
  hnd_write_request_func_t      write_work;
  /* a write operation was reported ready is now
     finally retrieved from the host and this
     callback notifies the handler that the associated
     buffer/SPI is free again.
  */
  hnd_write_done_func_t         write_done;

  /* a new mtu was set by the host.
     return accepted value
  */
  hnd_set_mtu_func_t            set_mtu;
};
typedef struct pamela_handler pamela_handler_t;
typedef const pamela_handler_t *pamela_handler_ptr_t;

/* ----- macros to help create handlers ----- */

#define HANDLER_BEGIN(name)          const pamela_handler_t name ROM_ATTR = {
#define HANDLER_END                  };

#define HANDLER_GET_PORT_BEGIN(hnd)  read_rom_word(&hnd->config.port_begin)
#define HANDLER_GET_PORT_END(hnd)    read_rom_word(&hnd->config.port_end)
#define HANDLER_GET_DEF_MTU(hnd)     read_rom_word(&hnd->config.def_mtu)
#define HANDLER_GET_MAX_MTU(hnd)     read_rom_word(&hnd->config.max_mtu)
#define HANDLER_GET_MAX_SLOTS(hnd)   read_rom_char(&hnd->config.max_slots)

#define HANDLER_FUNC_WORK(hnd)        ((hnd_work_func_t)read_rom_rom_ptr(&hnd->work))

#define HANDLER_FUNC_OPEN(hnd)        ((hnd_open_func_t)read_rom_rom_ptr(&hnd->open))
#define HANDLER_FUNC_OPEN_WORK(hnd)   ((hnd_open_func_t)read_rom_rom_ptr(&hnd->open_work))
#define HANDLER_FUNC_CLOSE(hnd)       ((hnd_close_func_t)read_rom_rom_ptr(&hnd->close))
#define HANDLER_FUNC_CLOSE_WORK(hnd)  ((hnd_close_func_t)read_rom_rom_ptr(&hnd->close_work))
#define HANDLER_FUNC_RESET(hnd)       ((hnd_reset_func_t)read_rom_rom_ptr(&hnd->reset))
#define HANDLER_FUNC_RESET_WORK(hnd)  ((hnd_reset_func_t)read_rom_rom_ptr(&hnd->reset_work))

#define HANDLER_FUNC_SEEK(hnd)        ((hnd_seek_func_t)read_rom_rom_ptr(&hnd->seek))
#define HANDLER_FUNC_TELL(hnd)        ((hnd_tell_func_t)read_rom_rom_ptr(&hnd->tell))

#define HANDLER_FUNC_READ_REQUEST(hnd ) ((hnd_read_request_func_t)read_rom_rom_ptr(&hnd->read_request))
#define HANDLER_FUNC_READ_WORK(hnd )    ((hnd_read_request_func_t)read_rom_rom_ptr(&hnd->read_work))
#define HANDLER_FUNC_READ_DONE(hnd)     ((hnd_read_done_func_t)read_rom_rom_ptr(&hnd->read_done))

#define HANDLER_FUNC_WRITE_REQUEST(hnd) ((hnd_write_request_func_t)read_rom_rom_ptr(&hnd->write_request))
#define HANDLER_FUNC_WRITE_WORK(hnd )   ((hnd_read_request_func_t)read_rom_rom_ptr(&hnd->write_work))
#define HANDLER_FUNC_WRITE_DONE(hnd)    ((hnd_write_done_func_t)read_rom_rom_ptr(&hnd->write_done))

#define HANDLER_FUNC_SET_MTU(hnd)       ((hnd_set_mtu_func_t)read_rom_rom_ptr(&hnd->set_mtu))

#endif
