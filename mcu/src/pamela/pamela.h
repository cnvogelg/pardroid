#ifndef PAMELA_H
#define PAMELA_H

#include "pamela_handler.h"
#include "pamela_shared.h"

/* number of channels used by pamela */
#ifndef PAMELA_NUM_CHANNELS
#define PAMELA_NUM_CHANNELS             16
#endif

/* max number of handlers used by pamela */
#ifndef PAMELA_NUM_HANDLERS
#define PAMELA_NUM_HANDLERS             8
#endif

/* default mtu */
#ifndef PAMELA_DEFAULT_MTU
#define PAMELA_DEFAULT_MTU              512
#endif

#define PAMELA_OK       0
#define PAMELA_ERROR    1

/* first time setup of pamela and all lower layers */
extern void pamela_init(void);

/* add a handler to pamela
   return PAMELA_OK or PAMELA_ERROR if too many handlers
*/
extern u08 pamela_add_handler(pamela_handler_ptr_t handler);

/* regular call in main loop to perform pamela's tasks */
extern void pamela_work(void);

// ----- API functions for handlers to use -----

/* the handler reports that the requested read operation
   is ready with the given parameters.
   A null pointer in the buffer denotes an SPI transfer.

   The buffer is filled with the data to be read by the host.
*/
extern void pamela_read_reply(u08 chn, u08 *buf, u16 size);

/* terminate the pending read request with an error */
extern void pamela_read_error(u08 chn);

/* the handler reports that the requested read operation
   is ready with the given parameters.
   A null pointer in the buffer denotes an SPI transfer.

   An empty buffer is passed that will be filled by the host.
*/
extern void pamela_write_reply(u08 chn, u08 *buf, u16 size);

/* terinate the pending write request with an error */
extern void pamela_write_error(u08 chn);

/* end the stream by the handler.
   either with an error or with regular EOS
 */
extern void pamela_end_stream(u08 chn, u08 error);

#endif
