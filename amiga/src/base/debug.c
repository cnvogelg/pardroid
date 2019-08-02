#include <proto/exec.h>
#include <stdarg.h>

#include "compiler.h"
#include "debug.h"

#ifdef __VBCC__

#define SIZE 80

struct buffer {
  UBYTE buf[SIZE];
  int pos;
};

#define SysBase (*(struct Library **)4)

ASM static void my_putch(REG(d0, char ch),
                         REG(a3, struct buffer *buf))
{
  if(buf->pos < SIZE) {
    buf->buf[buf->pos] = ch;
    buf->pos++;
  }
}

void KPrintF(char *fmt, ...)
{
  struct buffer buf;
  int i;
  va_list ap;

  buf.pos = 0;
  va_start(ap, fmt);
  RawDoFmt(fmt, ap, my_putch, &buf);
  va_end(ap);

  for(i=0;i<buf.pos;i++) {
    KPutCh(buf.buf[i]);
  }
}

#endif /* VBCC */
