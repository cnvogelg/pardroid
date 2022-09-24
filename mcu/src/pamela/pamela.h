#ifndef PAMELA_H
#define PAMELA_H

#include "pamela_handler.h"
#include "pamela/wire.h"

/* number of channels used by pamela */
#ifndef PAMELA_NUM_CHANNELS
#define PAMELA_NUM_CHANNELS             16
#endif

/* max number of handlers used by pamela */
#ifndef PAMELA_NUM_SERVICES
#define PAMELA_NUM_SERVICES             8
#endif

/* default mtu */
#ifndef PAMELA_DEFAULT_MTU
#define PAMELA_DEFAULT_MTU              512
#endif

#define PAMELA_OK 0
#define PAMELA_ERROR 1
#define PAMELA_BUSY 0xff

#define PAMELA_NO_SLOT       0xff
#define PAMELA_NO_SERVICE_ID 0xff

/* first time setup of pamela and all lower layers */
extern void pamela_init(void);

/* add a handler to pamela
   return service instance id or PAMELA_NO_SERVICE_ID
*/
extern u08 pamela_add_handler(pamela_handler_ptr_t handler);

/* regular call in main loop to perform pamela's tasks */
extern void pamela_work(void);

// ----- API functions for handlers to use -----

/* map a channel to a service slot */
extern u08 pamela_get_slot(u08 chn);
/* map a channel to a service instance */
extern u08 pamela_get_srv_id(u08 chn);

/* end the stream by the handler.
   either with an error or with regular EOS
 */
extern void pamela_end_stream(u08 chn, u08 error);

#endif
