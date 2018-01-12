#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_MEM

#include "types.h"
#include "mem.h"
#include "mem_int.h"
#include "debug.h"
#include "uart.h"
#include "uartutil.h"
#include "system.h"

#ifndef NULL
#define NULL (void *)0
#endif

struct mem_info {
    struct mem_info *prev;
    struct mem_info *next;
    u16 size;
};
typedef struct mem_info mem_info_t;

#define heap_end  (mem_end - CONFIG_STACK_SIZE)
#define guard_ptr (heap_end + 1)
#define max_free  (u16)(heap_end - mem_start)

// declare buffer storage
static u16 free_total;
static mem_info_t *first_mi;

void mem_init(void)
{
  mem_info_t *mi = (mem_info_t *)mem_start;
  mi->prev = NULL;
  mi->next = NULL;
  mi->size = max_free;

  first_mi = mi;
  free_total = max_free;
  DS("Bi:@"); DP(mem_start); DC('+'); DW(free_total); DNL;

  // place guard
  *(guard_ptr) = 0x42;
}

void mem_check(void)
{
  if(*guard_ptr != 0x42) {
    uart_send_pstring(PSTR("mem_check: GUARD FAILED!!\n"));
    system_sys_reset();
  }

  if(stack_pointer() <= guard_ptr) {
    uart_send_pstring(PSTR("mem_check: SP FAILED!!\n"));
    system_sys_reset();
  }
}

u16 mem_get_free(void)
{
  return free_total;
}

u08 *mem_alloc(u16 size)
{
  u16 real_size = size;

  /* store extra size word before allocation */
  size += 2;

  /* enforce min size so mem info fits always */
  if(size < sizeof(mem_info_t)) {
    size = sizeof(mem_info_t);
  }

  DNL;

  /* no memory free */
  if(free_total < size) {
    DS("{Ba!}"); DNL;
    return NULL;
  }

  /* search free chunk */
  mem_info_t *mi = first_mi;
  while(mi != NULL) {
    /* suitable? */
    if(mi->size >= size) {
      u16 remainder = mi->size - size;
      u08 *buf;
      /* use full mem info? */
      if(remainder < sizeof(mem_info_t)) {
        size = mi->size;
        /* unlink mem info */
        if(mi->prev != NULL) {
          mi->prev->next = mi->next;
        }
        if(mi->next != NULL) {
          mi->next->prev = mi->prev;
        }
        if(mi == first_mi) {
          first_mi = mi->next;
        }
        buf = ((u08 *)mi);
      }
      /* shrink mem info only */
      else {
        mi->size -= size;
        /* buffer is located after mem info */
        buf = ((u08 *)mi) + mi->size;
      }
      /* adjust total free */
      free_total -= size;
      /* store real size in buf */
      u16 *sp = (u16 *)buf;
      *sp = real_size;
      /* return buffer after size */
      DS("{Ba:@"); DP(buf + 2); DC('+'); DW(size); DC('='); DW(real_size); DC('}'); DNL;
      return buf + 2;
    }
    mi = mi->next;
  }
  /* nothing found */
  DS("{Ba?}"); DNL;
  return NULL;
}

static void free_internal(u08 *ptr, u16 size)
{
  /* create a new mem info there */
  mem_info_t *mi = (mem_info_t *)ptr;
  mi->size = size;
  free_total += size;

  /* no mem info */
  if(first_mi == NULL) {
    mi->prev = NULL;
    mi->next = NULL;
    first_mi = mi;
    DC('!'); DNL;
  } else {
    /* find mem info before (or NULL) */
    mem_info_t *prev_mi = first_mi;
    while(prev_mi != NULL) {
      mem_info_t *n = prev_mi->next;
      if((n == NULL) || (n > mi)) {
        break;
      }
      prev_mi = n;
    }
    if(prev_mi == NULL) {
      first_mi = mi;
    }
    DC('p'); DP(prev_mi);
    /* find mem info after (or NULL) */
    mem_info_t *next_mi;
    if(prev_mi == NULL) {
      next_mi = first_mi;
    } else {
      next_mi = prev_mi->next;
    }
    DC('n'); DP(next_mi);
    /* can we merge mem infos? */
    u08 merge_prev = 0;
    u08 merge_next = 0;
    DC(',');
    if(prev_mi != NULL) {
      u08 *ptr = (u08 *)prev_mi;
      DC('P'); DP(ptr + prev_mi->size);
      if((ptr + prev_mi->size) == (u08*)mi) {
        merge_prev = 1;
        DC('!');
      }
    }
    if(next_mi != NULL) {
      u08 *ptr = (u08 *)mi;
      DC('N'); DP(ptr + mi->size);
      if((ptr + mi->size) == (u08 *)next_mi) {
        merge_next = 1;
        DC('!');
      }
    }
    /* merge triple */
    if(merge_prev && merge_next) {
      DC('T');
      prev_mi->size += mi->size + next_mi->size;
      prev_mi->next = next_mi->next;
      if(next_mi->next != NULL) {
        next_mi->next->prev = prev_mi;
      }
    }
    /* merge prev and current */
    else if(merge_prev) {
      DC('<');
      prev_mi->size += mi->size;
      prev_mi->next = next_mi;
      if(next_mi != NULL) {
        next_mi->prev = prev_mi;
      }
    }
    /* merge current and next */
    else if(merge_next) {
      DC('>');
      mi->size += next_mi->size;
      mi->next = next_mi->next;
      if(next_mi->next != NULL) {
        next_mi->next->prev = mi;
      }
    }
    /* no merge possible. link new mi */
    else {
      DC('-');
      mi->prev = prev_mi;
      if(prev_mi != NULL) {
        prev_mi->next = mi;
      }
      mi->next = next_mi;
      if(next_mi != NULL) {
        next_mi->prev = mi;
      }
    }
  }

  DC('}'); DNL;
}

void mem_free(u08 *ptr)
{
  /* get pointer of size field */
  u08 *sp = ptr - 2;
  u16 real_size = *((u16 *)sp);

  /* adjust size */
  u16 size = real_size;
  size += 2;

  /* enforce min size so mem info fits always */
  if(size < sizeof(mem_info_t)) {
    size = sizeof(mem_info_t);
  }

  DNL;
  DS("{Bf:@"); DP(ptr); DC('+'); DW(size); DC('='); DW(real_size);
  free_internal(sp, size);
}

void mem_shrink(u08 *ptr, u16 new_real_size)
{
  /* get pointer of size field */
  u08 *sp = ptr - 2;
  u16 cur_real_size = *((u16 *)sp);

  /* nothing to do */
  if(new_real_size >= cur_real_size) {
    return;
  }

  /* adjust size */
  u16 cur_size = cur_real_size;
  cur_size += 2;

  /* enforce min size so mem info fits always */
  if(cur_size < sizeof(mem_info_t)) {
    cur_size = sizeof(mem_info_t);
  }

  u16 new_size = new_real_size;
  new_size += 2;
  if(new_size < sizeof(mem_info_t)) {
    new_size = sizeof(mem_info_t);
  }

  /* nothing to do */
  if(new_size >= cur_size) {
    return;
  }

  /* can't shrink: remainder is too small */
  u16 new_delta = cur_size - new_size;
  if(new_delta < sizeof(mem_info_t)) {
    return;
  }

  DNL;
  DS("{Bs:@"); DP(ptr); DC('+'); DW(cur_size); DC('='); DW(cur_real_size);
  DC('>'); DW(new_size); DC('='); DW(new_real_size);
  free_internal(sp + new_size, new_delta);

  /* store new size */
  *((u16 *)sp) = new_real_size;
}

