#ifndef DISK_MAP_H
#define DISK_MAP_H

#include "disk.h"

/* number of slots in disk map */
#ifndef DISK_MAP_SLOTS
#define DISK_MAP_SLOTS    4
#endif

#define DISK_MAP_INVALID_SLOT  0xff

struct disk_map_slot {
  u08     device;
  u08     unit;
  u08     flags;
};
typedef struct disk_map_slot disk_map_slot_t;

extern void disk_map_init(void);
extern u08  disk_map_get_slot(u08 slot, disk_map_slot_t *entry);
extern u08  disk_map_set_slot(u08 slot, disk_map_slot_t *entry);

extern u08  disk_map_open_slot(u08 slot);
extern u08  disk_map_close_slot(u08 slot);
extern disk_handle_t *disk_map_get_handle(u08 slot);

#endif
