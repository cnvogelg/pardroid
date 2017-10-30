#ifndef CHANNEL_H
#define CHANNEL_H

#include "reg.h"

extern void channel_init(void);
extern void channel_work(void);

/* handler API */
extern u08 channel_get_flags(u08 chn);
extern u08 channel_is_open(u08 chn);
extern u16 channel_get_mtu(u08 chn);

/* reg funcs */
extern void channel_reg_index(u16 *v,u08 mode);
extern void channel_reg_ctrl_status(u16 *v,u08 mode);
extern void channel_reg_mtu(u16 *v,u08 mode);

REG_TABLE_DECLARE(channel)

#endif
