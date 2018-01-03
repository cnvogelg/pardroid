#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_DRIVER_UFAT

#include "debug.h"
#include "ufat.h"

static u16 get_word(const u08 *buf, u16 offset)
{
  return (u16)buf[offset] | (u16)buf[offset+1] << 8;
}

static u32 get_long(const u08 *buf, u16 offset)
{
  return (u32)buf[offset] | (u32)buf[offset+1] << 8 |
         (u32)buf[offset+2] << 16 | (u32)buf[offset+3] << 24;
}

static u08 read_part_table(ufat_disk_t *disk, u32 *part_offset)
{
  // read first block
  u08 *buf = disk->tmp_buf;
  int res = ufat_io_read_block(0, buf);
  if(res != UFAT_RESULT_OK) {
    return res;
  }

  // check mbr signature
  u16 signature = get_word(buf, 510);
  DS("part sig:"); DW(signature); DNL;
  if(signature != 0xaa55) {
    return UFAT_RESULT_NO_PARTITION_TABLE;
  }

  // read first partition
  u16 p_off = 446;
  u08 p_type = buf[p_off+4];
  DS("type:"); DB(p_type); DNL;
  if((p_type != 11) && (p_type != 12)) {
    return UFAT_RESULT_WRONG_PARTITION_TYPE;
  }

  // get offset of partition
  u32 offset = get_long(buf, p_off+8);
  DS("off:"); DL(offset); DNL;
  *part_offset = offset;

  return UFAT_RESULT_OK;
}

static u08 read_boot_sector(ufat_disk_t *disk, u32 part_offset)
{
  // read boot block
  u08 *buf = disk->tmp_buf;
  int res = ufat_io_read_block(part_offset, buf);
  if(res != UFAT_RESULT_OK) {
    return res;
  }

  // check signature
  u16 signature = get_word(buf, 510);
  if(signature != 0xaa55) {
    return UFAT_RESULT_NO_FAT_BOOT_BLOCK;
  }

  // read params
  u16 bytes_per_sec = get_word(buf, 11);
  DS("bytes_per_sec:"); DW(bytes_per_sec); DNL;
  if(bytes_per_sec != 512) {
    return UFAT_RESULT_UNSUPPORTED_FAT;
  }

  disk->sec_per_clus = buf[13];
  DS("sec_per_clus:"); DW(disk->sec_per_clus); DNL;
  u16 reserved_secs = get_word(buf, 14);
  DS("reserved_secs:"); DW(reserved_secs); DNL;
  u08 num_fats = buf[16];
  DS("num_fats:"); DB(num_fats); DNL;
  u16 root_entry_count = get_word(buf, 17);
  DS("root_entry_count:"); DW(root_entry_count); DNL;
  u16 total_sec16 = get_word(buf, 19);
  DS("total_sec16:"); DW(total_sec16); DNL;
  u16 fat_size16 = get_word(buf, 22);
  DS("fat_size16:"); DW(fat_size16); DNL;
  u32 total_sec32 = get_long(buf, 32);
  DS("total_sec32:"); DL(total_sec32); DNL;

  u32 fat_size;
  u32 root_clus;
  u32 total_sec;
  if(total_sec16 == 0) {
    total_sec = total_sec32;
  } else {
    total_sec = total_sec16;
  }
  if(fat_size16 == 0) {
    fat_size = get_long(buf, 36);
    root_clus = get_long(buf, 44);
    disk->flags = UFAT_FLAG_FAT32;
    DS("--FAT32"); DNL;
  } else {
    fat_size = fat_size16;
    root_clus = 2;
    disk->flags = UFAT_FLAG_FAT16;
    DS("--FAT16"); DNL;
  }
  DS("total_sec:"); DL(total_sec); DNL;
  DS("fat_size:"); DL(fat_size); DNL;
  DS("root_clus:"); DL(root_clus); DNL;

  // calc derived values
  DS("--"); DNL;
  disk->fat_start = part_offset + reserved_secs;
  disk->fat_secs  = fat_size * num_fats;
  DS("fat_start:"); DL(disk->fat_start); DNL;
  DS("fat_secs:"); DL(disk->fat_secs); DNL;
  disk->root_start = disk->fat_start + disk->fat_secs;
  disk->root_secs  = (32 * root_entry_count) >> 9; // /512
  disk->data_start = disk->root_start + disk->root_secs;
  disk->data_secs  = total_sec - disk->data_start;
  if(disk->flags == UFAT_FLAG_FAT32) {
    // adjust FAT32 root dir location
    disk->root_start = disk->data_start + (root_clus - 2) * disk->sec_per_clus;
    disk->root_secs = disk->sec_per_clus; // first cluster for now
  }
  DS("root_start:"); DL(disk->root_start); DNL;
  DS("root_secs:"); DL(disk->root_secs); DNL;
  DS("data_start:"); DL(disk->data_start); DNL;
  DS("data_secs:"); DL(disk->data_secs); DNL;

  disk->num_clus = disk->data_secs / disk->sec_per_clus;
  DS("num_clus:"); DL(disk->num_clus); DNL;

  return UFAT_RESULT_OK;
}

u08 ufat_disk_init(ufat_disk_t *disk)
{
  u32 part_offset;
  u08 res = read_part_table(disk, &part_offset);
  if(res != UFAT_RESULT_OK) {
    return res;
  }

  return read_boot_sector(disk, part_offset);
}

static void copy_sfn_name(const u08 *in, u08 *out)
{
  const u08 *ext = in + 8;
  for(u08 i=0;i<8;i++) {
    if(*in == ' ') {
      break;
    }
    *out++ = *in++;
  }
  *out++ = '.';
  for(u08 i=0;i<3;i++) {
    if(*ext == ' ') {
      break;
    }
    *out++ = *ext++;
  }
  *out = '\0';
}

static u08 dir_scan(ufat_disk_t *disk, ufat_dir_entry_t *de, ufat_scan_func_t func,
                    u32 lba, u32 num, void *user_data)
{
  u08 is_last = 0;
  u08 *buf = disk->tmp_buf;
  while((num > 0) && (!is_last)) {
    // read dir block
    DS("dir block:"); DL(lba); DNL;
    int res = ufat_io_read_block(lba, buf);
    if(res != UFAT_RESULT_OK) {
      return res;
    }

    // FAT has 16 dir entries a 32 byte per 512 block
    u08 *e = buf;
    for(u08 i=0;i<16;i++) {
      DC('#'); DB(i); DC(':');
      u08 first_char = e[0];
      // dir entry is marked as free
      if(first_char == 0xe5) {
        DS("free"); DNL;
        goto next_entry;
      }
      // get entry attributes
      u08 attr = e[11];
      // last dir entry
      if(attr == 0) {
        is_last = 1;
        DS("last"); DNL;
        break;
      }
      // ignore high bits
      attr &= 0x1f;
      // skip lfn entries
      if(attr == 0xf) {
        DS("LFN"); DNL;
        goto next_entry;
      }
      // skip hidden
      if(attr & 2) {
        DS("hidden"); DNL;
        goto next_entry;
      }
      // skip volume
      if(attr & 8) {
        DS("volume"); DNL;
        goto next_entry;
      }

      // got valid entry
      if(attr & 0x10) {
        de->type = UFAT_TYPE_DIR;
        DS("DIR:");
      } else {
        de->type = UFAT_TYPE_FILE;
        DS("file:");
      }
      u16 clus_hi = get_word(e, 20);
      u16 clus_lo = get_word(e, 26);
      de->size_bytes = get_long(e, 28);
      de->start_clus = (u32)clus_hi << 16 | (u32)clus_lo;
      DS("size:"); DL(de->size_bytes);
      DS("clus:"); DL(de->start_clus);

      copy_sfn_name(e, de->name);
      DC(':'); DSB(de->name); DNL;

      // call func
      u08 stay = func(de, user_data);
      if(stay != UFAT_SCAN_CONTINUE) {
        is_last = 1;
        break;
      }

next_entry:
      e += 32;
    }
  }
  return UFAT_RESULT_OK;
}

u08 ufat_root_scan(ufat_disk_t *disk, ufat_dir_entry_t *de,
                   ufat_scan_func_t func, void *user_data)
{
  u32 lba = disk->root_start;
  u32 num = disk->root_secs;
  return dir_scan(disk, de, func, lba, num, user_data);
}

static u08 to_upper(u08 u)
{
  if((u >= 'a') && (u <= 'z')) {
    return u - 32;
  } else {
    return u;
  }
}

u08 ufat_name_match(const ufat_dir_entry_t *de, const u08 *name)
{
  const u08 *fat_name = de->name;
  u08 a = 0;
  do {
    a = to_upper(*name++);
    u08 b = *fat_name++;
    if(a != b) {
      return 0;
    }
  } while(a != 0);
  return 1;
}

struct match_data {
  const u08 *name;
  u08 found;
};

static u08 match_func(const ufat_dir_entry_t *de, void *user_data)
{
  struct match_data *md = (struct match_data *)user_data;
  if(ufat_name_match(de, md->name)) {
    md->found = 1;
    return UFAT_SCAN_STOP;
  } else {
    return UFAT_SCAN_CONTINUE;
  }
}

u08 ufat_root_find(ufat_disk_t *disk, ufat_dir_entry_t *de, const u08 *name)
{
  u32 lba = disk->root_start;
  u32 num = disk->root_secs;
  struct match_data md = {
    name, 0
  };
  u08 res = dir_scan(disk, de, match_func, lba, num, &md);
  if(res != UFAT_RESULT_OK) {
    return res;
  }
  if(md.found) {
    return UFAT_RESULT_OK;
  } else {
    return UFAT_RESULT_ENTRY_NOT_FOUND;
  }
}
