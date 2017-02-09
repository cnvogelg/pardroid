#include <proto/exec.h>

#define __NOLIBBASE__
#include <proto/dos.h>

extern int dosmain(void);

struct Library *DOSBase;

int main(void)
{
  int res = RETURN_ERROR;
  if (DOSBase = OpenLibrary ("dos.library",36)) {
    res = dosmain();
    CloseLibrary (DOSBase);
  }
  return res;
}
