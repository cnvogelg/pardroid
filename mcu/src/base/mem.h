#ifndef MEM_H
#define MEM_H

extern void mem_init(void);
extern void mem_check(void);
extern u16  mem_get_free(void);
extern u08 *mem_alloc(u16 size);
extern void mem_free(u08 *ptr);
extern void mem_shrink(u08 *ptr, u16 new_size);

#endif
