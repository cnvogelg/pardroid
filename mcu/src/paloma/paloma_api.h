#ifndef PALOMA_API_H
#define PALOMA_API_H

#include "paloma/types.h"

extern void paloma_api_param_all_reset(void);
extern void paloma_api_param_all_load(void);
extern void paloma_api_param_all_save(void);

extern u08 paloma_api_param_get_total_slots();
extern u08 paloma_api_param_get_type(u08 slot);
extern u08 paloma_api_param_get_id(u08 slot);
extern void paloma_api_param_get_min_max_bytes(u08 slot, u08 *min, u08 *max);

extern void paloma_api_param_get_info(u08 slot, paloma_param_info_t *info);

extern u08  paloma_api_param_get_ubyte(u08 slot, u08 def);
extern void paloma_api_param_set_ubyte(u08 slot, u08 val);

extern u16  paloma_api_param_get_uword(u08 slot, u08 def);
extern void paloma_api_param_set_uword(u08 slot, u16 val);

extern u32  paloma_api_param_get_ulong(u08 slot, u08 def);
extern void paloma_api_param_set_ulong(u08 slot, u32 val);

extern u08  paloma_api_param_get_buffer(u08 slot, u08 *data);
extern void paloma_api_param_set_buffer(u08 slot, u08 *data, u08 size);

#endif
