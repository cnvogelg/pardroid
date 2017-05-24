#ifndef DEBUG_H
#define DEBUG_H

/* VBCC */
void __KPutCh(__reg("a6") void *, __reg("d0") UBYTE ch) = "\tjsr\t-516(a6)";
#define KPutCh(ch) __KPutCh(SysBase, ch)


#ifdef KDEBUG

extern void KPrintF(char *, ...);
#define D(x) KPrintF x ;

#else

#define D(x)

#endif /* CONFIG_DEBUG */

#endif /* DEBUG_H */
