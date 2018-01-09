#ifndef MEM_INT_H
#define MEM_INT_H

#include <avr/io.h>

extern char __heap_start;

/* pointer after last RAM cell */
#define mem_end   ((char *)(RAMEND + 1))
#define mem_start (&__heap_start)

#endif
