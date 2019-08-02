#ifndef COMPILER_H
#define COMPILER_H

/* compiler specific switches */
#ifdef __VBCC__
#define REG(r,t) __reg( #r ) t
#define SAVEDS __saveds
#define ASM
#else
#ifdef __GNUC__
#define REG(r,t) t __asm(#r)
/* only for baserel. #define SAVEDS __attribute__((saveds)) */
#define SAVEDS
#define ASM
#else
#error unsupported compiler
#endif /* GNUC */
#endif /* VBCC */

#endif /* COMPILER_H */
