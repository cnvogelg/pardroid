#ifndef MEM_INT_H
#define MEM_INT_H

extern char __bss_end;
extern char __estack;

#define mem_end    (&__estack)
#define mem_start  (&__bss_end)

#endif
