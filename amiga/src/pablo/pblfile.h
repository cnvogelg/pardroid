#ifndef PBLFILE_H
#define PBLFILE_H

struct pblfile {
  ULONG   rom_size;
  UWORD   fw_id;
  UWORD   version;
  UWORD   mach_tag;
  UWORD   pad;
  UBYTE   *data;
};

typedef struct pblfile pblfile_t;

#define PBLFILE_OK            0
#define PBLFILE_ERROR_OPEN    1
#define PBLFILE_ERROR_TAG     2
#define PBLFILE_ERROR_HEADER  3
#define PBLFILE_ERROR_DATA    4
#define PBLFILE_ERROR_NOMEM   5
#define PBLFILE_ERROR_MACHTAG 6
#define PBLFILE_ERROR_CRC     7

extern int pblfile_load(const char *name, pblfile_t *pf);
extern void pblfile_free(pblfile_t *pf);
extern int pblfile_check(pblfile_t *pf);
extern const char *pblfile_perror(int status);

#endif
