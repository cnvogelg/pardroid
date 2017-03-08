#ifndef REG_RO_H
#define REG_RO_H

#define reg_ro_get  proto_api_get_ro_reg

extern u16 reg_ro_get(u08 num);

// define this in your code:
extern u08 reg_ro_size(void);
extern const u16 reg_ro_table[] ROM_ATTR;

#endif
