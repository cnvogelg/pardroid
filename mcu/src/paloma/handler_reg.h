#ifndef HANDLER_REG_H
#define HANDLER_REG_H

#include "reg.h"

#define HANDLER_REG_CONTROL_OPEN       0
#define HANDLER_REG_CONTROL_CLOSE      1

extern void handler_reg_init(void);

/* reg funcs */
extern void handler_reg_index(u16 *v,u08 mode);
extern void handler_reg_ctrl_status(u16 *v,u08 mode);
extern void handler_reg_mtu(u16 *v,u08 mode);

REG_TABLE_DECLARE(handler)

#endif
