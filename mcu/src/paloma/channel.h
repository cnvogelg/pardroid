#ifndef CHANNEL_H
#define CHANNEL_H

#define CHANNEL_INVALID        0xff

extern void channel_init(void);
extern u08  channel_alloc(u08 id);
extern void channel_free(u08 chn);
extern u08  channel_get_id(u08 chn);

#endif
