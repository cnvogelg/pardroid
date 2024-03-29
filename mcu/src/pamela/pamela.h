#ifndef PAMELA_H
#define PAMELA_H

#include "pamela_handler.h"
#include "pamela_int.h"
#include "pamela/wire.h"

/* number of channels used by pamela */
#ifndef PAMELA_NUM_CHANNELS
#define PAMELA_NUM_CHANNELS             16
#endif

/* default mtu */
#ifndef PAMELA_DEFAULT_MTU
#define PAMELA_DEFAULT_MTU              512
#endif

/* handler result values
   if its another value it is assumed to be an error code
   typically a PAMELA_WIRE_ERROR_*
*/
#define PAMELA_HANDLER_OK 0
#define PAMELA_HANDLER_POLL 0xff

/* handler state values
   first call is PAMELA_CALL_fIRST then PAMELA_CALL_nEXT
*/
#define PAMELA_CALL_FIRST 0
#define PAMELA_CALL_NEXT  1


#define PAMELA_NO_SLOT       0xff
#define PAMELA_NO_SERVICE_ID 0xff

/* task control flags */
#define PAMELA_TASK_ON 1
#define PAMELA_TASK_OFF 0

/* first time setup of pamela and all lower layers */
extern void pamela_init(void);
/* regular call in main loop to perform pamela's tasks */
extern void pamela_work(void);

// ----- API functions for handlers to use -----

/* map a channel to a service slot */
extern u08 pamela_get_slot(u08 chn);
/* map a channel to a service instance */
extern u08 pamela_get_srv_id(u08 chn);
/* map a channel to a handler */
extern pamela_handler_ptr_t pamela_get_handler(u08 chn);

/* end the stream by the handler.
   either with an error or with regular EOS
 */
extern void pamela_end_stream(u08 chn, u08 error);

/* ----- tasks ----- */
/* toggle channel task PAMELA_ON or PAMELA_OFF */
extern void pamela_channel_task_control(u08 chn, u08 on);

#endif
