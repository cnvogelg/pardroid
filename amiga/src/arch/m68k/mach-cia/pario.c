#define __NOLIBBASE__
#include <proto/exec.h>
#include <proto/misc.h>

#include <resources/misc.h>
#include <resources/cia.h>

#include "debug.h"
#include "pario.h"

struct pario_handle {
  ULONG initFlags;
  struct Library *sysBase;
  struct Library *miscBase;
  struct Libary *ciaABase;
};

#define MiscBase ph->miscBase
#define CIAABase ph->ciaABase

const char *pario_tag = "pario";

struct pario_handle *pario_init(struct Library *SysBase)
{
  /* alloc handle */
  struct pario_handle *ph;
  ph = AllocMem(sizeof(struct pario_handle), MEMF_CLEAR);
  if(ph == NULL) {
    return NULL;
  }
  ph->sysBase = SysBase;

  /* get misc.resource */
  D(("OpenResouce(MISCNAME)\n"));
  MiscBase = OpenResource(MISCNAME);
  if(MiscBase != NULL) {

    /* get ciaa.resource */
    D(("OpenResource(CIANAME)\n"));
    CIAABase = OpenResource(CIAANAME);
    if(CIAABase != NULL) {

      /* obtain exclusive access to the parallel hardware */
      if (!AllocMiscResource(MR_PARALLELPORT, pario_tag)) {
        ph->initFlags = 1;
        if (!AllocMiscResource(MR_PARALLELBITS, pario_tag)) {
          ph->initFlags = 3;
          /* ok */
          return ph;
        }
      }
    }
  }

  /* something failed. clean up */
  pario_exit(ph);
  return NULL;
}

#define SysBase ph->sysBase

void pario_exit(struct pario_handle *ph)
{
  if(ph == NULL) {
    return;
  }

  /* free resources */
  if(ph->initFlags & 1) {
    FreeMiscResource(MR_PARALLELPORT);
  }
  if(ph->initFlags & 2) {
    FreeMiscResource(MR_PARALLELBITS);
  }

  /* free handle */
  FreeMem(ph, sizeof(struct pario_handle));
}
