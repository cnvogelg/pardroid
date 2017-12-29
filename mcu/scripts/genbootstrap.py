#!/usr/bin/env python3
# Usage:
#  genbootstrap.py <in_file> <out_header> <name> [ver_maj] [ver_min]
#
# create the bootstrap header file

import os
import sys
import struct

# get params
nargs = len(sys.argv)
if nargs != 5 and nargs != 6:
  print("Usage: <in_data> <out.c> <out.h> <name> [out_knok]")
  sys.exit(1)

in_bin = sys.argv[1]
out_src = sys.argv[2]
out_hdr = sys.argv[3]
name = sys.argv[4]
if nargs == 6:
  knok_file = sys.argv[5]
else:
  knok_file = None

up_name = name.upper()
low_name = name.lower()

# read input rom image
with open(in_bin, "rb") as fh:
  rom_data = fh.read()

size = len(rom_data)
print("size:  %04x" % size)

# append header?
# +0: KNOK
# +4: size word
# +6: check sum word
# =8
if knok_file is not None:
  # pad to long
  rem = size % 4
  if rem != 0:
    add = 4 - rem
    size += add
    rom_data += "\x00" * size
    print("pad:", size)
  # calc check sum
  num_words = size // 2
  pos = 0
  check_sum = 0
  for n in range(num_words):
    l = struct.unpack_from(">H", rom_data, pos)[0]
    check_sum = (check_sum + l) & 0xffff
    pos += 2
  print("check: %04x" % check_sum)
  # create header
  hdr_data = b"KNOK" + struct.pack(">HH", size, check_sum)
  rom_data = hdr_data + rom_data
  total_size = len(rom_data)
  header_size = len(hdr_data)
  # dump file
  with open(knok_file, "wb") as fh:
    fh.write(rom_data)
else:
  total_size = size
  header_size = 0

print("hdr:   %04x" % header_size)
print("total: %04x" % total_size)

# write header
with open(out_hdr, "w") as fh:
  # header
  fh.write("""/* AUTOMATICALLY GENERATED bootstrap code! DO NOT MODIFY!! */
#ifndef {0}_H
#define {0}_H

#define {0}_HEADER_SIZE  {2}
#define {0}_DATA_SIZE    {1}
extern const u08 {4}_code[{3}] ROM_ATTR;

#endif
""".format(up_name, size, header_size, total_size, low_name))

# write source
with open(out_src, "w") as fh:
  fh.write("""/* AUTOMATICALLY GENERATED bootstrap code! DO NOT MODIFY!! */

#include "autoconf.h"
#include "types.h"
#include "arch.h"

const u08 {1}_code[{0}] ROM_ATTR =
""".format(total_size, low_name))
  fh.write("{\n")
  # data
  cnt = 0
  for d in rom_data:
    fh.write("0x%02x, " % d)
    cnt += 1
    if cnt == 16:
      fh.write("\n")
      cnt = 0
  # footer
  fh.write("\n};\n");

# done
sys.exit(0)
