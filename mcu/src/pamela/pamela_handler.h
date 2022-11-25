#ifndef PAMELA_HANDLER_H
#define PAMELA_HANDLER_H

/* data buffer used in pamela */
struct pamela_buf {
  u08  *data;
  u16   size;
};
typedef struct pamela_buf pamela_buf_t;

/* ----- open/close/reset ----- */
typedef u08 (*hnd_open_func_t)(u08 chn, u16 port);
typedef u08 (*hnd_close_func_t)(u08 chn);
typedef u08 (*hnd_reset_func_t)(u08 chn);

/* ----- reconfig ----- */
typedef u16 (*hnd_set_mtu_func_t)(u08 chn, u16 mtu);

/* ----- seek/tell ----- */
typedef void (*hnd_seek_func_t)(u08 chn, u32 pos);
typedef u32 (*hnd_tell_func_t)(u08 chn);

/* ----- read/write ----- */
typedef u08 (*hnd_read_func_t)(u08 chn, pamela_buf_t *buf);
typedef u08 (*hnd_write_func_t)(u08 chn, pamela_buf_t *buf);

/* ----- tasks ----- */
typedef void (*hnd_service_task_func_t)(u08 srv_id);
typedef void (*hnd_channel_task_func_t)(u08 chn);

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

     if PAMELA_POLL is returned then the
     open operation takes some time and
     is continued with open_work_func.
  */
  hnd_open_func_t               open;
  /* continue opening port
     return PAMELA_OK or error to finish the open
     operation. use PAMELA_POLL to continue
     getting calls.

     only required if open returns PAMELA_POLL
  */
  hnd_open_func_t               open_poll;

  /* close the channel. similar to open
     return PAMELA_OK or PAMELA_POLL
  */
  hnd_close_func_t              close;
  /* close cont.
     return PAMELA_OK or PAMELA_POLL
  */
  hnd_close_func_t              close_poll;

  /* reset the channel.
     this keeps the channel open but all
     pending reads/writes are cancelled
     and any error flags are reset.
     return PAMELA_OK or PAMELA_POLL
  */
  hnd_reset_func_t              reset;
  /* cont. reset
     return PAMELA_OK or PAMELA_POLL
  */
  hnd_reset_func_t              reset_poll;

  /* seek to a given position */
  hnd_seek_func_t               seek;
  /* return current seek position */
  hnd_tell_func_t               tell;

  /* ----- read ----- */
  /* a read request arrived from the host with the
     desired max size to read in buf->size.

     you have to either accept the size or reduce
     it to the available data size.

     additionally, you have to fill the buffer
     pointer in buf->data.

     if data and size is in place then return
     PAMELA_OK. if you need to wait for incoming
     data to be read then return PAMELA_POLL.
     it will call read_poll() until PAMELA_OK
     is returned.

     if reading is not possible then return
     PAMELA_ERROR.
  */
  hnd_read_func_t       read_request;
  /* continue read operation
     return PAMELA_OK when done, an error or PAMELA_POLL
     to continue
  */
  hnd_read_func_t       read_poll;
  /* a read operation that was reported ready is now
     finally retrieved from the host and this
     callback notifies the handler that the associated
     buffer/SPI is free again.
  */
  hnd_read_func_t          read_done;

  /* ----- write ----- */
  /* a write request arrived from the host with the
     desired size to read.

     either accept the size of reduce it if needed.
     additionally, set the buffer to be used for
     retrieval.

     return PAMELA_OK to start writing or
     use PAMELA_POLL to delay the request.
     it will call write_poll() until PAMELA_OK is
     returned.

     return PAMELA_ERROR if writing is not possible.
  */
  hnd_write_func_t      write_request;
  /* continue WRITE operation
     return PAMELA_OK when done, an error or BUSY
     to continue
  */
  hnd_write_func_t      write_poll;
  /* a write operation was reported ready is now
     finally retrieved from the host and this
     callback notifies the handler that the associated
     buffer/SPI is free again.
  */
  hnd_write_func_t         write_done;

  /* a new mtu was set by the host.
     return accepted value
  */
  hnd_set_mtu_func_t            set_mtu;

  /* ----- tasks ----- */
  /* an optional task function that will be called for the service */
  hnd_service_task_func_t       service_task;
  /* an optional task function that will be called for an active channel */
  hnd_channel_task_func_t       channel_task;
};
typedef struct pamela_handler pamela_handler_t;
typedef const pamela_handler_t *pamela_handler_ptr_t;

/* ----- macros to help create handlers ----- */

#define HANDLER_DECLARE(name) \
  extern const pamela_handler_t name ROM_ATTR;

#define HANDLER_BEGIN(name)          const pamela_handler_t name ROM_ATTR = {
#define HANDLER_END                  };

#define HANDLER_GET_PORT_BEGIN(hnd)  read_rom_word(&hnd->config.port_begin)
#define HANDLER_GET_PORT_END(hnd)    read_rom_word(&hnd->config.port_end)
#define HANDLER_GET_DEF_MTU(hnd)     read_rom_word(&hnd->config.def_mtu)
#define HANDLER_GET_MAX_MTU(hnd)     read_rom_word(&hnd->config.max_mtu)
#define HANDLER_GET_MAX_SLOTS(hnd)   read_rom_char(&hnd->config.max_slots)

#define HANDLER_FUNC_WORK(hnd)        ((hnd_work_func_t)read_rom_rom_ptr(&hnd->work))

#define HANDLER_FUNC_OPEN(hnd)        ((hnd_open_func_t)read_rom_rom_ptr(&hnd->open))
#define HANDLER_FUNC_OPEN_POLL(hnd)   ((hnd_open_func_t)read_rom_rom_ptr(&hnd->open_poll))
#define HANDLER_FUNC_CLOSE(hnd)       ((hnd_close_func_t)read_rom_rom_ptr(&hnd->close))
#define HANDLER_FUNC_CLOSE_POLL(hnd)  ((hnd_close_func_t)read_rom_rom_ptr(&hnd->close_poll))
#define HANDLER_FUNC_RESET(hnd)       ((hnd_reset_func_t)read_rom_rom_ptr(&hnd->reset))
#define HANDLER_FUNC_RESET_POLL(hnd)  ((hnd_reset_func_t)read_rom_rom_ptr(&hnd->reset_poll))

#define HANDLER_FUNC_SEEK(hnd)        ((hnd_seek_func_t)read_rom_rom_ptr(&hnd->seek))
#define HANDLER_FUNC_TELL(hnd)        ((hnd_tell_func_t)read_rom_rom_ptr(&hnd->tell))

#define HANDLER_FUNC_READ_REQUEST(hnd ) ((hnd_read_func_t)read_rom_rom_ptr(&hnd->read_request))
#define HANDLER_FUNC_READ_POLL(hnd )    ((hnd_read_func_t)read_rom_rom_ptr(&hnd->read_poll))
#define HANDLER_FUNC_READ_DONE(hnd)     ((hnd_read_func_t)read_rom_rom_ptr(&hnd->read_done))

#define HANDLER_FUNC_WRITE_REQUEST(hnd) ((hnd_write_func_t)read_rom_rom_ptr(&hnd->write_request))
#define HANDLER_FUNC_WRITE_POLL(hnd )   ((hnd_write_func_t)read_rom_rom_ptr(&hnd->write_poll))
#define HANDLER_FUNC_WRITE_DONE(hnd)    ((hnd_write_func_t)read_rom_rom_ptr(&hnd->write_done))

#define HANDLER_FUNC_SET_MTU(hnd)       ((hnd_set_mtu_func_t)read_rom_rom_ptr(&hnd->set_mtu))

#define HANDLER_FUNC_SERVICE_TASK(hnd)  ((hnd_service_task_func_t)read_rom_rom_ptr(&hnd->service_task))
#define HANDLER_FUNC_CHANNEL_TASK(hnd)  ((hnd_channel_task_func_t)read_rom_rom_ptr(&hnd->channel_task))

/* ----- macros to create the handler table ----- */

#define HANDLER_TABLE_DECLARE \
  extern const pamela_handler_ptr_t handler_table[] ROM_ATTR; \
  extern const u08 handler_table_size ROM_ATTR; \
  extern pamela_service_t pamela_services[];

#define HANDLER_TABLE_BEGIN   const pamela_handler_ptr_t handler_table[] ROM_ATTR = {

#define HANDLER_TABLE_END     }; \
  const u08 handler_table_size ROM_ATTR = sizeof(handler_table) / sizeof(pamela_handler_ptr_t); \
  pamela_service_t pamela_services[sizeof(handler_table) / sizeof(pamela_handler_ptr_t)];

#define HANDLER_TABLE_GET_SIZE()   read_rom_char(&handler_table_size)

#define HANDLER_TABLE_GET_ENTRY(x) (pamela_handler_ptr_t)read_rom_rom_ptr(&handler_table[x]);

#endif
