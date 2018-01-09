#ifndef MEM_INT_H
#define MEM_INT_H

extern char __bss_end;
extern char _estack;

#define mem_end    (&_estack)
#define mem_start  (&__bss_end)

static inline char *stack_pointer(void)
{
  char *stack;
  __asm__ volatile("mov %0, sp" : "=r" (stack) ::);
  return stack;
}

#endif
