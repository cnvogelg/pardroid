#ifndef PAMELA_REQ_HANDLER_H
#define PAMELA_REQ_HANDLER_H

#include "pamela_req_int.h"

/* ----- handler functions ----- */

/* begin request */
typedef u08 (*hnd_req_begin_func_t)(u08 chan, u08 **buf_in, u16 size_in);

/* process the request */
typedef u08 (*hnd_req_handle_func_t)(u08 chan, u08 **buf_io, u16 *size_io);

/* the request is done  */
typedef void (*hnd_req_end_func_t)(u08 chan, u08 *buf_out, u16 size_out);

// ROM handler of command
struct pamela_req_handler {
  /* (internal) pointer to slots for request handling */
  pamela_req_slot_t      *slots;
  /* (internal) pointer to pamela handler */
  pamela_handler_ptr_t    handler;

  /* begin a request and prepare the request buffer for the incoming request
     and return it. pamela will fill the request buffer and continue
     with handle(). */
  hnd_req_begin_func_t     begin;

  /* the request buffer is filled now and passed via buf_io/size_io.

     You could now either keep the input buffer and write the reply on
     top of it. adjust only the size if necessary.

     Or you switch the buffer and adjust the pointer and the size if
     necessary.

     If processing takes more time then return PAMELA_POLL and you will
     be triggered with handle_poll() again until PAMELA_OK or PAMELA_ERROR
     is returned. */
  hnd_req_handle_func_t    handle;

  /* for long running operations call the handle_poll() */
  hnd_req_handle_func_t    handle_poll;

  /* finally a request is completed when the response was sent and the
     response buffer is free to be used again. */
  hnd_req_end_func_t      end;
};
typedef struct pamela_req_handler pamela_req_handler_t;
typedef const pamela_req_handler_t *pamela_req_handler_ptr_t;

/* ----- macros for req handler ----- */

#define REQ_HANDLER_BEGIN(name, port, num_slots, mtu) \
    const pamela_handler_t name ## _srv ROM_ATTR = { \
      .config.port_begin = port, \
      .config.port_end = port, \
      .config.def_mtu = mtu, \
      .config.max_mtu = mtu, \
      .config.max_slots = num_slots, \
      .open = pamela_req_open, \
      .close = pamela_req_close, \
      .reset = pamela_req_reset, \
      .read_request = pamela_req_read_request, \
      .read_done = pamela_req_read_done, \
      .write_request = pamela_req_write_request, \
      .write_done = pamela_req_write_done, \
      .channel_task = pamela_req_channel_task, \
    }; \
    pamela_req_slot_t name ## _slots[num_slots]; \
    const pamela_req_handler_t name ROM_ATTR = { \
        .slots = name ## _slots, \
        .handler = & name ## _srv, \

#define REQ_HANDLER_END };

#define REQ_HANDLER_GET_SLOTS(hnd)   (pamela_req_slot_t *)read_rom_ram_ptr(&hnd->slots)
#define REQ_HANDLER_GET_HANDLER(hnd) (pamela_handler_ptr_t)read_rom_rom_ptr(&hnd->handler)

#define REQ_HANDLER_FUNC_BEGIN(hnd)       ((hnd_req_begin_func_t)read_rom_rom_ptr(&hnd->begin))
#define REQ_HANDLER_FUNC_HANDLE(hnd)      ((hnd_req_handle_func_t)read_rom_rom_ptr(&hnd->handle))
#define REQ_HANDLER_FUNC_HANDLE_POLL(hnd) ((hnd_req_handle_func_t)read_rom_rom_ptr(&hnd->handle_poll))
#define REQ_HANDLER_FUNC_END(hnd)         ((hnd_req_end_func_t)read_rom_rom_ptr(&hnd->end))

/* ----- macros for req handler table ----- */

#define REQ_HANDLER_TABLE_DECLARE \
  extern const pamela_req_handler_ptr_t req_handler_table[] ROM_ATTR; \
  extern const u08 req_handler_table_size ROM_ATTR; \

#define REQ_HANDLER_TABLE_BEGIN   const pamela_req_handler_ptr_t req_handler_table[] ROM_ATTR = {

#define REQ_HANDLER_TABLE_END     }; \
  const u08 req_handler_table_size ROM_ATTR = sizeof(req_handler_table) / sizeof(pamela_req_handler_ptr_t); \

#define REQ_HANDLER_TABLE_GET_SIZE()   read_rom_char(&req_handler_table_size)

#define REQ_HANDLER_TABLE_GET_ENTRY(x) (pamela_req_handler_ptr_t)read_rom_rom_ptr(&req_handler_table[x]);

#define ADD_REQ_HANDLER(name)   &name ## _srv

#endif
