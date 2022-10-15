#ifndef pamela_req_HANDLER_H
#define pamela_req_HANDLER_H

#include "pamela_req_int.h"

/* ----- handler functions ----- */

typedef u08 (*hnd_req_begin_func_t)(u08 chan, u08 **req_buf, u16 req_size);

/* process the request and return the filled reply buffer and size */
typedef u08 (*hnd_req_handle_func_t)(u08 chan, u08 *req_buf, u16 req_size, u08 **rep_buf, u16 *rep_size);

/* the request is done  */
typedef void (*hnd_req_end_func_t)(u08 chan, u08 *rep_buf, u16 rep_size);

// ROM handler of command
struct pamela_req_handler {
  /* begin a request and prepare the request buffer for the incoming request
     and return it. pamela will fill the request buffer and continue
     with process(). */
  hnd_req_begin_func_t     begin;

  /* the request buffer is filled and you have to process the request and
     prepare a reply for it. Fill and return the reply buffer.

     You may use the same buffer for request and response.
     Then simply return the req_buf as rep_buf as well.

     If processing takes more time then return PAMELA_BUSY and you will
     be triggered with handle_work() untiel PAMELA_OK or PAMELA_ERROR
     is returned. */
  hnd_req_handle_func_t    handle;

  /* for long running operations call the handle_work() */
  hnd_req_handle_func_t    handle_work;

  /* finally a request is completed when the response was sent and the
     response buffer is free to be used again. */
  hnd_req_end_func_t      end;
};
typedef struct pamela_req_handler pamela_req_handler_t;
typedef const pamela_req_handler_t *pamela_req_handler_ptr_t;


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
    }; \
    pamela_req_slot_t name ## _slots[num_slots]; \
    const pamela_req_handler_t name ROM_ATTR = {

#define REQ_HANDLER_END };

#define REQ_HANDLER_ADD(name) \
    pamela_req_add_handler(&name ## _srv, \
                           &name, \
                           name ## _slots)

#define REQ_HANDLER_FUNC_BEGIN(hnd)       ((hnd_req_begin_func_t)read_rom_rom_ptr(&hnd->begin))
#define REQ_HANDLER_FUNC_HANDLE(hnd)      ((hnd_req_handle_func_t)read_rom_rom_ptr(&hnd->handle))
#define REQ_HANDLER_FUNC_HANDLE_WORK(hnd) ((hnd_req_handle_func_t)read_rom_rom_ptr(&hnd->handle_work))
#define REQ_HANDLER_FUNC_END(hnd)         ((hnd_req_end_func_t)read_rom_rom_ptr(&hnd->end))

#endif
