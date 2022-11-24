#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_DISK

#include "debug.h"

#include "disk_map.h"

struct disk_map_entry {
  disk_handle_t     handle;
  u08               open_status;
};
typedef struct disk_map_entry disk_map_entry_t;

/* the static disk map */
static disk_map_entry_t disk_map[DISK_MAP_SLOTS];

void disk_map_init(void)
{
  /* prefill all entries with DEVICE_NONE */
  disk_map_entry_t *entry = disk_map;
  for(u08 i=0;i<DISK_MAP_SLOTS;i++) {
    entry->open_status = DISK_ERROR_NO_DEVICE;
    entry++;
  }
}

u08 disk_map_get_slot(u08 slot, disk_map_slot_t *entry)
{
  if(slot >= DISK_MAP_SLOTS) {
    return DISK_ERROR_INVALID_SLOT;
  }

  disk_map_entry_t *map = &disk_map[slot];
  entry->device = map->handle.device;
  entry->unit = map->handle.unit;
  entry->flags = map->handle.flags;
  return DISK_OK;
}

u08 disk_map_set_slot(u08 slot, disk_map_slot_t *entry)
{
  if(slot >= DISK_MAP_SLOTS) {
    return DISK_ERROR_INVALID_SLOT;
  }

  disk_map_entry_t *map = &disk_map[slot];
  map->handle.device = entry->device;
  map->handle.unit = entry->unit;
  map->handle.flags = entry->flags;
  return DISK_OK;
}

u08 disk_map_open_slot(u08 slot)
{
  if(slot >= DISK_MAP_SLOTS) {
    return DISK_ERROR_INVALID_SLOT;
  }

  disk_map_entry_t *map = &disk_map[slot];

  /* slot already open? */
  if(map->open_status == DISK_OK) {
    return DISK_ERROR_ALREADY_OPEN;
  }

  u08 status = disk_open(&map->handle, map->handle.device, map->handle.unit, map->handle.flags);
  map->open_status = status;
  return status;
}

u08 disk_map_close_slot(u08 slot)
{
  if(slot >= DISK_MAP_SLOTS) {
    return DISK_ERROR_INVALID_SLOT;
  }

  disk_map_entry_t *map = &disk_map[slot];

  /* slot not open? */
  if(map->open_status != DISK_OK) {
    return DISK_ERROR_NOT_OPEN;
  }

  disk_close(&map->handle);
  return DISK_OK;
}

disk_handle_t *disk_map_get_handle(u08 slot)
{
  if(slot >= DISK_MAP_SLOTS) {
    return NULL;
  }

  disk_map_entry_t *map = &disk_map[slot];
  if(map->open_status != DISK_OK) {
    return NULL;
  }

  return &map->handle;
}


