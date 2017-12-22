#!/usr/bin/env python3
# Usage:
#  genbootstrap.py <in_file> <out_header>
#
# create the bootstrap header file

import os
import sys

# get params
if len(sys.argv) != 4:
  print("Usage: <in_data> <out.c> <out.h>")
  sys.exit(1)

in_bin = sys.argv[1]
out_src = sys.argv[2]
out_hdr = sys.argv[3]

# read input rom image
with open(in_bin, "rb") as fh:
  rom_data = fh.read()

size = len(rom_data)
print("size:", size)

# write header
with open(out_hdr, "w") as fh:
  # header
  fh.write("""/* AUTOMATICALLY GENERATED bootstrap code! DO NOT MODIFY!! */
#ifndef BOOTSTRAP_H
#define BOOTSTRAP_H

extern const u08 bootstrap_code[%d] ROM_ATTR;

#endif
""" % size)

# write source
with open(out_src, "w") as fh:
  fh.write("""/* AUTOMATICALLY GENERATED bootstrap code! DO NOT MODIFY!! */

#include "autoconf.h"
#include "types.h"
#include "arch.h"

const u08 bootstrap_code[%d] ROM_ATTR = {
""" % size)
  # data
  cnt = 0
  for d in rom_data:
    fh.write("0x%02x, " % d)
    cnt += 1
    if cnt == 16:
      fh.write("\n")
      cnt = 0
  # footer
  fh.write("""};
""")

# done
sys.exit(0)
