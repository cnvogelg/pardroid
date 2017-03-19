#!/usr/bin/env python3
# Usage:
#  pblfile.py in.bin max_size out.pbl
#
# add the PBL1 file header to a pablo ROM image

import os
import sys
import struct

# get params
if len(sys.argv) != 6:
  print("Usage: <in.img> <max_size> <mach_tag> <version_tag> <out.pbl>")
  sys.exit(1)

in_bin = sys.argv[1]
max_size = int(sys.argv[2])
mach_tag = int(sys.argv[3][2:],16)
version_tag = int(sys.argv[4][2:],16)
out_pbl = sys.argv[5]

# read input rom image
with open(in_bin, "rb") as fh:
  rom_data = fh.read()

# check size
if len(rom_data) != max_size:
  print("INVALID SIZE!")
  sys.exit(1)

# add pablo file header (big endian):
# +0: PBL1          magic
# +4: size (ULONG)  size of ROM image
# +8: version       version tag
# +10: mach tag     machine identification
# #12: total
pbl_hdr = b'PBL1' + struct.pack(">IHH", max_size, version_tag, mach_tag)

# write imaget
with open(out_pbl, "wb") as fh:
  fh.write(pbl_hdr)
  fh.write(rom_data)

# done
sys.exit(0)
