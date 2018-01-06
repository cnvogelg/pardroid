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
  u08 res = ufat_io_read_block(0, buf);
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
  u08 res = ufat_io_read_block(part_offset, buf);
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
    disk->fat_shift = 2; // cluster * 4 for FAT offset
  } else {
    disk->fat_shift = 1; // cluster * 2 for FAT offset
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
    u08 res = ufat_io_read_block(lba, buf);
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

/* ----- read file ----- */

u08 ufat_read_file_init(const ufat_disk_t *disk, const ufat_dir_entry_t *de,
                        ufat_read_file_t *rf)
{
  rf->left_bytes = de->size_bytes;
  rf->cur_clus = de->start_clus;
  rf->cur_lba = disk->data_start + (de->start_clus - 2) * disk->sec_per_clus;
  rf->next_clus = de->start_clus;
  rf->cur_sec = 0;
  rf->num_cont_clus = 0;
  return UFAT_RESULT_OK;
}

static u08 find_next_cluster(const ufat_disk_t *disk, ufat_read_file_t *rf)
{
  u32 clus = rf->next_clus;

  DS("--find_next: clus="); DL(clus); DNL;

  // calc fat location of cluster
  u32 fat_byte_off = clus << disk->fat_shift;
  u16 blk_off = (u16)(fat_byte_off & 0x1ff);
  u32 fat_blk = fat_byte_off >> 9; // div by 512 sector size
  fat_blk += disk->fat_start;

  // read fat block
  u08 *buf = disk->tmp_buf;
  DS("fat block:"); DL(fat_blk); DS(",off="); DW(blk_off); DNL;
  u08 res = ufat_io_read_block(fat_blk, buf);
  if(res != UFAT_RESULT_OK) {
    return res;
  }

  // scan fat block
  u32  cur_clus = get_long(buf, blk_off);

  // sanity check
  if(disk->flags & UFAT_FLAG_FAT32) {
    if(cur_clus == 0x0ffffff8) {
      DS("EOC32??");
      return UFAT_RESULT_EARLY_CHAIN_END;
    }
  } else {
    if(cur_clus == 0xfff8) {
      DS("EOC16??");
      return UFAT_RESULT_EARLY_CHAIN_END;
    }
  }

  // setup new current cluster/lba
  rf->cur_clus = cur_clus;
  rf->cur_lba = disk->data_start + (cur_clus - 2) * disk->sec_per_clus;
  DS("new cur_clus:"); DL(rf->cur_clus); DNL;
  DS("new cur_lba:"); DL(rf->cur_lba); DNL;

  // scan cluster chain if it is continous in current block?
  u08 num_cont_clus = 0;
  blk_off += 1 << disk->fat_shift;
  while(blk_off < 512) {
    u32 next_clus = get_long(buf, blk_off);
    // next cluster is not consecutive
    if(next_clus != cur_clus + 1) {
      cur_clus = next_clus;
      break;
      DC('!');
    }
    num_cont_clus++;
    cur_clus = next_clus;
    blk_off += 1 << disk->fat_shift;
    DC('.');
  }
  DNL;

  DS("num_cont_clus:"); DB(num_cont_clus); DNL;
  DS("next_clus:"); DL(cur_clus); DNL;

  rf->num_cont_clus = num_cont_clus;
  rf->next_clus = cur_clus;

  return UFAT_RESULT_OK;
}

u08 ufat_read_file_next(const ufat_disk_t *disk, ufat_read_file_t *rf,
                        u16 *size)
{
  // nothing to do?
  if(rf->left_bytes == 0) {
    *size = UFAT_READ_FILE_EOF;
    return UFAT_RESULT_OK;
  }

#if 0
  // state dump
  DS("left_bytes:"); DL(rf->left_bytes); DNL;
  DS("cur_lba:"); DL(rf->cur_lba); DNL;
  DS("cur_clus:"); DL(rf->cur_clus); DNL;
  DS("next_clust:"); DL(rf->next_clus); DNL;
  DS("cur_sec:"); DB(rf->cur_sec); DNL;
  DS("num_cont_clus:"); DB(rf->num_cont_clus); DNL;
#endif

  // save data lba
  u32 lba = rf->cur_lba;

  // adjust left size
  if(rf->left_bytes <= 512) {
    *size = UFAT_READ_FILE_EOF | (u16)(rf->left_bytes);
    rf->left_bytes = 0;
  }
  // regular intermediate block
  else {
    *size = 512;
    rf->left_bytes -= 512;

    rf->cur_sec++;
    // last sector in cluster consumed?
    if(rf->cur_sec == disk->sec_per_clus) {
      // reset sector count
      rf->cur_sec = 0;
      // now search next cluster:
      // still continous clusters available?
      if(rf->num_cont_clus > 0) {
        rf->cur_clus++;
        rf->cur_lba++;
        rf->num_cont_clus--;
      }
      // no, read FAT to find out following clusters
      else {
        u08 res = find_next_cluster(disk, rf);
        if(res != UFAT_RESULT_OK) {
          return res;
        }
      }
    }
    // no, stay in cluster
    else {
      rf->cur_lba++;
    }
  }

  // finally read block
#if 0
  DS("data block:"); DL(lba); DS(","); DL(rf->left_bytes); DNL;
#endif
  return ufat_io_read_block(lba, disk->tmp_buf);
}

/* ----- block i/o ------ */

static u08 build_clu_map(const ufat_disk_t *disk, ufat_blk_io_t *bio, u32 cluster)
{
  struct clu_map *clu_map = &bio->clu_map[0];

  // init first entry
  clu_map->cluster = cluster;
  clu_map->num_cont_clus = 0;

  // mini map with a single cluster only?
  if(bio->num_blks <= disk->sec_per_clus) {
    DS("mini map!"); DNL;
    return UFAT_RESULT_OK;
  }

  u32 fat_byte_off;
  u32 blk_off;
  u32 fat_blk;
  u08 num = 0;

new_fat_block:
  // prepare to read FAT block containing the first cluster
  // and calc fat location of cluster in block
  fat_byte_off = cluster << disk->fat_shift;
  blk_off = (u16)(fat_byte_off & 0x1ff);
  fat_blk = fat_byte_off >> 9; // div by 512 sector size
  fat_blk += disk->fat_start;

  while(1) {
    // read fat block
    u08 *buf = disk->tmp_buf;
    DS("fat block:"); DL(fat_blk); DS(",off="); DW(blk_off); DNL;
    u08 res = ufat_io_read_block(fat_blk, buf);
    if(res != UFAT_RESULT_OK) {
      return res;
    }

    // scan cluster chain of current FAT block
    // if it is continous in current block?
    DS("CUR:@"); DL(cluster);
    while(blk_off < 512) {
      u32 next_clus = get_long(buf, blk_off);

      // end of chain?
      if((next_clus & 0x0ffffff8) == 0x0ffffff8) {
        DS("EOC"); DW(clu_map->num_cont_clus); DNL;
        return UFAT_RESULT_OK;
      }

      // next cluster is not consecutive
      if(next_clus != cluster + 1) {
        DW(clu_map->num_cont_clus); DNL;
        DS("NEW:@"); DL(next_clus); DNL;
        // no more cluster entries?
        if(num == UFAT_MAX_BLK_IO_ENTRIES) {
          DS("MAP TOO SMALL!"); DNL;
          return UFAT_RESULT_BLK_IO_MAP_TOO_SMALL;
        }
        // create new
        clu_map++;
        num++;
        clu_map->cluster = next_clus;
        clu_map->num_cont_clus = 0;
        cluster = next_clus;
        goto new_fat_block;
      }
      // current entry is full
      else if(clu_map->num_cont_clus == UFAT_CLU_MAP_MAX_ENTRY) {
        DS("FULL"); DW(clu_map->num_cont_clus); DNL;
        if(num == UFAT_MAX_BLK_IO_ENTRIES) {
          DS("MAP TOO SMALL");
          return UFAT_RESULT_BLK_IO_MAP_TOO_SMALL;
        }
        // create new
        clu_map++;
        num++;
        clu_map->cluster = next_clus;
        clu_map->num_cont_clus = 0;
      }
      // update current
      else {
        clu_map->num_cont_clus++;
        DC('.');
      }

      // next entry in fat block
      cluster = next_clus;
      blk_off += 1 << disk->fat_shift;
    }

    // end of current block reached... read next fat block
    DS("NEXT FAT"); DNL;
    fat_blk++;
    blk_off = 0;
  }
  return UFAT_RESULT_OK;
}

u08 ufat_blk_io_init(const ufat_disk_t *disk, const ufat_dir_entry_t *de,
                     ufat_blk_io_t *bio)
{
  /* check file size */
  u32 size = de->size_bytes;
  /* enforce block alignment */
  if((size & 0x1ff) != 0) {
    return UFAT_RESULT_WRONG_SIZE_FOR_BLK_IO;
  }
  bio->num_blks = size >> 9;
  bio->num_cluster = (bio->num_blks + disk->sec_per_clus - 1) / disk->sec_per_clus;
  bio->sec_in_last = bio->num_blks % disk->sec_per_clus;
  DS("blk_io_init:num_blks="); DL(bio->num_blks);
  DS(",num_cluster:"); DL(bio->num_cluster);
  DS(",sec_in_last:"); DW(bio->sec_in_last); DNL;

  /* setup cluster map */
  return build_clu_map(disk, bio, de->start_clus);
}

u08 ufat_blk_io_map(const ufat_disk_t *disk, ufat_blk_io_t *bio,
                    u32 blk_no, u32 *lba)
{
  if(blk_no >= bio->num_blks) {
    return UFAT_RESULT_INVALID_BLK_NO;
  }

  // calc clu no and sec in cluster of file
  u32 clu_no = blk_no / disk->sec_per_clus;
  u32 sec = blk_no % disk->sec_per_clus;
  DS("io_map:clu_no="); DL(clu_no); DS(",sec="); DL(sec); DNL;

  // scan throug map
  struct clu_map *clu_map = &bio->clu_map[0];
  u32 cluster = 0;
  for(u08 num = 0;num < UFAT_MAX_BLK_IO_ENTRIES;num++) {
    DS(" clu_map#"); DB(num); DC(':'); DL(clu_map->cluster);
    DC('+'); DW(clu_map->num_cont_clus); DNL;

    /* does fit in current range? */
    u16 cont_clus = clu_map->num_cont_clus;
    if(clu_no <= cont_clus) {
      cluster = clu_map->cluster + clu_no;
      // calc lba
      *lba = disk->data_start + (cluster - 2) * disk->sec_per_clus;
      DS("clu"); DL(cluster); DS("->lba="); DL(*lba); DNL;
      return UFAT_RESULT_OK;
    }

    /* next entry */
    clu_no -= cont_clus + 1;
    clu_map++;
  }

  // no more cluster entries but still not found!
  DS("too small??");
  return UFAT_RESULT_BLK_IO_MAP_TOO_SMALL;
}

u08 ufat_blk_io_read(const ufat_disk_t *disk, ufat_blk_io_t *bio,
                     u32 blk_no, u08 *buf)
{
  u32 lba = 0;
  u08 res = ufat_blk_io_map(disk, bio, blk_no, &lba);
  if(res != UFAT_RESULT_OK) {
    return res;
  }
  return ufat_io_read_block(lba, buf);
}

u08 ufat_blk_io_write(const ufat_disk_t *disk, ufat_blk_io_t *bio,
                             u32 blk_no, const u08 *buf)
{
  u32 lba = 0;
  u08 res = ufat_blk_io_map(disk, bio, blk_no, &lba);
  if(res != UFAT_RESULT_OK) {
    return res;
  }
  return ufat_io_write_block(lba, buf);
}
