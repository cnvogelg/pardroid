#include <inttypes.h>
#include <stdio.h>

typedef uint16_t u16;
typedef char *rom_pchar;
#define PSTR(x) x

#include "machtag.h"

int main(int argc, char **argv) {
  printf("0x%04x\n", MACHTAG);
  return 0;
}
