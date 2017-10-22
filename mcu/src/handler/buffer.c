#include "types.h"
#include "autoconf.h"
#include "buffer.h"
#include "debug.h"

#define NULL (void *)0

struct mem_info {
    struct mem_info *prev;
    struct mem_info *next;
    u16 size;
};
typedef struct mem_info mem_info_t;

// declare buffer storage
static u08 data[CONFIG_BUFFER_SIZE];
static u16 free_total;
static mem_info_t *first_mi;

void buffer_init(void)
{
  DS("Bi"); DNL;
  mem_info_t *mi = (mem_info_t *)data;
  mi->prev = NULL;
  mi->next = NULL;
  mi->size = CONFIG_BUFFER_SIZE;

  first_mi = mi;
  free_total = CONFIG_BUFFER_SIZE;
}

u16 buffer_get_free(void)
{
  return free_total;
}

u08 *buffer_alloc(u16 size)
{
  /* store extra size word before allocation */
  size += 2;

  /* enforce min size so mem info fits always */
  if(size < sizeof(mem_info_t)) {
    size = sizeof(mem_info_t);
  }

#if 0
  /* align to 4 bytes */
  if(size & 3) {
    size = (size & ~3) + 4;
  }
#endif

  /* no memory free */
  if(free_total < size) {
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
      /* store size in buf */
      u16 *sp = (u16 *)buf;
      *sp = size;
      /* return buffer after size */
      return buf + 2;
    }
    mi = mi->next;
  }
  /* nothing found */
  return NULL;
}

void buffer_free(u08 *ptr)
{
  /* get pointer of size field */
  u08 *sp = ptr - 2;
  u16 size = *((u16 *)sp);

  /* create a new mem info there */
  mem_info_t *mi = (mem_info_t *)sp;
  mi->size = size;
  free_total += size;

  /* no mem info */
  if(first_mi == NULL) {
    first_mi = mi;
  } else {
    /* find mem info before (or NULL) */
    mem_info_t *prev_mi = first_mi;
    while((prev_mi != NULL) && (prev_mi < mi)) {
      prev_mi = prev_mi->next;
    }
    if(prev_mi == NULL) {
      first_mi = mi;
    }
    /* find mem info after (or NULL) */
    mem_info_t *next_mi;
    if(prev_mi == NULL) {
      next_mi = first_mi;
    } else {
      next_mi = prev_mi->next;
    }
    /* can we merge mem infos? */
    u08 merge_prev = 0;
    u08 merge_next = 0;
    if((prev_mi != NULL) && ((prev_mi + prev_mi->size) == mi)) {
      merge_prev = 1;
    }
    if((next_mi != NULL) && ((mi + mi->size) == next_mi)) {
      merge_next = 1;
    }
    /* merge triple */
    if(merge_prev && merge_next) {
      prev_mi->size += mi->size + next_mi->size;
      prev_mi->next = next_mi->next;
      if(next_mi->next != NULL) {
        next_mi->next->prev = prev_mi;
      }
    }
    /* merge prev and current */
    else if(merge_prev) {
      prev_mi->size += mi->size;
      prev_mi->next = next_mi;
      if(next_mi != NULL) {
        next_mi->prev = prev_mi;
      }
    }
    /* merge current and next */
    else if(merge_next) {
      mi->size += next_mi->size;
      mi->next = next_mi->next;
      if(next_mi->next != NULL) {
        next_mi->next->prev = mi;
      }
    }
    /* no merge possible. link new mi */
    else {
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
}

