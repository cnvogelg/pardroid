#ifndef BUFFER_H
#define BUFFER_H

extern void buffer_init(void);
extern u16  buffer_get_free(void);
extern u08 *buffer_alloc(u16 size);
extern void buffer_free(u08 *ptr);
extern void buffer_shrink(u08 *ptr, u16 new_size);

#endif
