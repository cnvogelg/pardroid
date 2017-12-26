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
if nargs != 5 and nargs != 8:
  print("Usage: <in_data> <out.c> <out.h> <name> [ver_maj] [ver_min] [out_knok]")
  sys.exit(1)

in_bin = sys.argv[1]
out_src = sys.argv[2]
out_hdr = sys.argv[3]
name = sys.argv[4]
if nargs == 8:
  version = (int(sys.argv[5]), int(sys.argv[6]))
  knok_file = sys.argv[7]
else:
  version = None
  knok_file = None

up_name = name.upper()
low_name = name.lower()

# read input rom image
with open(in_bin, "rb") as fh:
  rom_data = fh.read()

size = len(rom_data)
print("size:  %08x" % size)

# append header?
# +0: KNOK
# +4: size
# +8: check
# +12: ver_maj
# +14: ver_min
# =16
if version is not None:
  # pad to long
  rem = size % 4
  if rem != 0:
    add = 4 - rem
    size += add
    rom_data += "\x00" * size
    print("pad:", size)
  # calc check sum
  num_longs = size // 4
  pos = 0
  check_sum = 0
  for n in range(num_longs):
    l = struct.unpack_from(">I", rom_data, pos)[0]
    check_sum = (check_sum + l) & 0xffffffff
    pos +=4
  print("check: %08x" % check_sum)
  # create header
  hdr_data = b"KNOK" + struct.pack(">IIHH", size, check_sum, version[0], version[1])
  rom_data = hdr_data + rom_data
  total_size = len(rom_data)
  header_size = len(hdr_data)
else:
  total_size = size
  header_size = 0

print("hdr:   %08x" % header_size)
print("total: %08x" % total_size)

# dump knok file
if knok_file is not None:
  with open(knok_file, "wb") as fh:
    fh.write(rom_data)

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
