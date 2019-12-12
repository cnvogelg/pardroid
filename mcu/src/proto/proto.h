#ifndef PROTO_H

#include "proto_shared.h"

extern void proto_init(void);
extern void proto_first_cmd(void);

extern void proto_handle(void);
extern void proto_handle_mini(void);

// send /ack irq signal to Amiga (1us pulse)
extern void proto_trigger_signal(void);

// control busy line
extern void proto_set_busy(void);
extern void proto_clr_busy(void);
extern u08 proto_is_busy(void);

#endif
