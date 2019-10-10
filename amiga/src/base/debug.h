#ifndef DEBUG_H
#define DEBUG_H

#ifdef __VBCC__

/* VBCC */
void __KPutCh(__reg("a6") void *, __reg("d0") UBYTE ch) = "\tjsr\t-516(a6)";
#define KPutCh(ch) __KPutCh(SysBase, ch)
extern void KPrintF(char *, ...);

#else
#ifdef __GNUC__

/* GCC */
#include <clib/debug_protos.h>

#endif /* GCC */
#endif /* VBCC */

#ifdef KDEBUG

#define D(x) KPrintF x ;

#else

#define D(x)

#endif /* KDEBUG */

#endif /* DEBUG_H */
