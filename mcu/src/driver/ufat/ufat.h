#ifndef UFAT_H
#define UFAT_H

#define UFAT_RESULT_OK                    0
#define UFAT_RESULT_READ_ERROR            1
#define UFAT_RESULT_WRITE_ERROR           2
#define UFAT_RESULT_NO_PARTITION_TABLE    3
#define UFAT_RESULT_WRONG_PARTITION_TYPE  4
#define UFAT_RESULT_NO_FAT_BOOT_BLOCK     5
#define UFAT_RESULT_UNSUPPORTED_FAT       6
#define UFAT_RESULT_ENTRY_NOT_FOUND       7
#define UFAT_RESULT_EARLY_CHAIN_END       8
#define UFAT_RESULT_WRONG_SIZE_FOR_BLK_IO 9
#define UFAT_RESULT_INVALID_BLK_NO        10
#define UFAT_RESULT_BLK_IO_MAP_TOO_SMALL  11

/* external API has to be defined */
extern u08 ufat_io_read_block(u32 lba, u08 *data);
extern u08 ufat_io_write_block(u32 lba, const u08 *data);

#define UFAT_FLAG_FAT16                   0
#define UFAT_FLAG_FAT32                   1

#define UFAT_SCAN_CONTINUE                0
#define UFAT_SCAN_STOP                    1

#define UFAT_TYPE_FILE                    0
#define UFAT_TYPE_DIR                     1

#define UFAT_READ_FILE_EOF                0x8000
#define UFAT_READ_FILE_SIZE_MASK          0x7fff

#define UFAT_CLU_MAP_MAX_ENTRY            0xffff

struct ufat_disk {
  u08  *tmp_buf;
  u32   fat_start;
  u32   fat_secs;
  u32   root_start;
  u32   root_secs;
  u32   data_start;
  u32   data_secs;
  u32   num_clus;
  u16   sec_per_clus;
  u08   flags;
  u08   fat_shift;
};
typedef struct ufat_disk ufat_disk_t;

struct ufat_dir_entry {
  u08   type;
  u08   name[13];
  u32   start_clus;
  u32   size_bytes;
};
typedef struct ufat_dir_entry ufat_dir_entry_t;

struct ufat_read_file {
  u32   left_bytes;     // in file
  u32   cur_lba;        // lba block number of current sector
  u32   cur_clus;       // current cluster number
  u32   next_clus;      // after the continous cluster this is the next one
  u08   cur_sec;        // in current cluster
  u08   num_cont_clus;  // how many continous clusters following?
};
typedef struct ufat_read_file ufat_read_file_t;

#define UFAT_MAX_BLK_IO_ENTRIES           3

struct ufat_blk_io {
  u32   num_blks;
  u32   num_cluster;
  u16   sec_in_last;
  struct clu_map {
    u32  cluster;
    u16  num_cont_clus;
  } clu_map[UFAT_MAX_BLK_IO_ENTRIES];
};
typedef struct ufat_blk_io ufat_blk_io_t;

typedef u08 (*ufat_scan_func_t)(const ufat_dir_entry_t *e, void *user_data);

/* public API */
extern u08 ufat_disk_init(ufat_disk_t *disk);
extern u08 ufat_root_scan(ufat_disk_t *disk, ufat_dir_entry_t *de,
                          ufat_scan_func_t func, void *user_data);
extern u08 ufat_root_find(ufat_disk_t *disk, ufat_dir_entry_t *de,
                          const u08 *name);
extern u08 ufat_name_match(const ufat_dir_entry_t *de, const u08 *name);

/* file read API */
extern u08 ufat_read_file_init(const ufat_disk_t *disk, const ufat_dir_entry_t *de,
                               ufat_read_file_t *rf);
extern u08 ufat_read_file_next(const ufat_disk_t *disk, ufat_read_file_t *rf,
                               u16 *size);

/* random access block I/O */
extern u08 ufat_blk_io_init(const ufat_disk_t *disk, const ufat_dir_entry_t *de,
                            ufat_blk_io_t *bio);
extern u08 ufat_blk_io_map(const ufat_disk_t *disk, ufat_blk_io_t *bio,
                           u32 blk_no, u32 *lba);
extern u08 ufat_blk_io_read(const ufat_disk_t *disk, ufat_blk_io_t *bio,
                            u32 blk_no, u08 *buf);
extern u08 ufat_blk_io_write(const ufat_disk_t *disk, ufat_blk_io_t *bio,
                             u32 blk_no, const u08 *buf);

#endif
