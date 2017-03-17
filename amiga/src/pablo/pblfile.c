#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include "pblfile.h"
#include "machtag.h"

#define PBL1_TAG    0x50424c31  // PBL1

int pblfile_load(const char *name, pblfile_t *pf)
{
  BPTR fh;
  ULONG tag;

  pf->data = 0;

  // open
  fh = Open(name, MODE_OLDFILE);
  if(fh == 0) {
    return PBLFILE_ERROR_OPEN;
  }

  // read tag
  if(FRead(fh, &tag, 4, 1) != 1) {
    Close(fh);
    return PBLFILE_ERROR_TAG;
  }

  // read header
  if(FRead(fh, pf, sizeof(pblfile_t)-4, 1) != 1) {
    Close(fh);
    return PBLFILE_ERROR_HEADER;
  }

  // alloc data
  pf->data = AllocVec(pf->rom_size, 0);
  if(pf->data == 0) {
    return PBLFILE_ERROR_NOMEM;
  }

  // load data
  if(FRead(fh, pf->data, pf->rom_size, 1) != 1) {
    Close(fh);
    return PBLFILE_ERROR_DATA;
  }

  Close(fh);
  return PBLFILE_OK;
}

void pblfile_free(pblfile_t *pf)
{
  if(pf->data != 0) {
    FreeVec(pf->data);
    pf->data = 0;
  }
}

// def crc16_ccitt(buf):
//   crc = 0xffff
//   for d in buf:
//     lo8 = crc & 0xff
//     hi8 = (crc >> 8) & 0xff
//     d ^= lo8
//     d ^= d << 4
//     a = d << 8 | hi8
//     crc = (a ^ (d >> 4) ^ (d << 3)) & 0xffff
//   return crc

static UWORD crc16_ccitt(const UBYTE *buf, ULONG size)
{
  UWORD crc = 0xffff;
  for(ULONG i=0;i<size;i++) {
    UBYTE data = *(buf++);
    UBYTE lo8 = (UBYTE)(crc & 0xff);
    UBYTE hi8 = (UBYTE)(crc >> 8);
    data ^= lo8;
    data ^= (UBYTE)(data << 4);

    UWORD a = (UWORD)data << 8 | hi8;
    crc = a ^ ((UWORD)data >> 4) ^ ((UWORD)data << 3);
  }
  return crc;
}

int pblfile_check(pblfile_t *pf)
{
  if(pf->data == 0) {
    return PBLFILE_ERROR_DATA;
  }

  int result = 0;
  UWORD arch = pf->mach_tag & MACHTAG_ARCH_MASK;
  switch(arch) {
    case MACHTAG_ARCH_AVR:
    {
      UWORD crc = crc16_ccitt(pf->data, pf->rom_size);
      result = (crc == 0) ? PBLFILE_OK : PBLFILE_ERROR_CRC;
      break;
    }
    default:
      result = PBLFILE_ERROR_MACHTAG;
      break;
  }
  return result;
}

const char *pblfile_perror(int status)
{
  switch(status) {
    case PBLFILE_OK:
      return "ok";
    case PBLFILE_ERROR_OPEN:
      return "failed opening file!";
    case PBLFILE_ERROR_TAG:
      return "no PBL tag found!";
    case PBLFILE_ERROR_HEADER:
      return "error reading header!";
    case PBLFILE_ERROR_DATA:
      return "error reading data!";
    case PBLFILE_ERROR_NOMEM:
      return "out of memory!";
    case PBLFILE_ERROR_MACHTAG:
      return "unknown machine tag!";
    case PBLFILE_ERROR_CRC:
      return "checksum error!";
    default:
      return "?";
  }
}
