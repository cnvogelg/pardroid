#!/usr/bin/env python3
# Usage:
#  pblfile.py in.bin max_size out.pbl
#
# add the PBL1 file header to a pablo ROM image

import os
import sys
import struct

# get params
if len(sys.argv) != 4:
  print("Usage: <in.img> <max_size> <out.pbl>")
  sys.exit(1)

in_bin = sys.argv[1]
max_size = int(sys.argv[2])
out_pbl = sys.argv[3]

# read input rom image
with open(in_bin, "rb") as fh:
  rom_data = fh.read()

# check size
if len(rom_data) != max_size:
  print("INVALID SIZE!")
  sys.exit(1)

# find fw info in ROM
pos = rom_data.find(b'FWID')
if pos > 0:
  fw_info_fmt = ">HHH" # big endian
else:
  pos = rom_data.find(b'DIWF')
  if pos > 0:
    fw_info_fmt = "<HHH" # little endian
  else:
    print("NO FWID found!!")
    sys.exit(1)
# skip magic
pos += 4
fw_id, version_tag, mach_tag = struct.unpack_from(fw_info_fmt, rom_data, pos)

# add pablo file header (big endian):
# +0: PBL1                magic
# +4: size (ULONG)        size of ROM image
# +8: fw_id (WORD)        firmware id
# +10: version (WORD)     version tag
# +12: mach tag (WORD)    machine identification
# +14: padding (WORD)
# #16: total
pbl_hdr = b'PBL1' + struct.pack(">IHHHH", max_size, fw_id, version_tag, mach_tag, 0)

# write image
with open(out_pbl, "wb") as fh:
  fh.write(pbl_hdr)
  fh.write(rom_data)

out_file = os.path.basename(out_pbl)
print("%-16s  SIZE=%06x  FW=%04x  VER=%04x  MACH=%04x" % \
  (out_file, max_size, fw_id, version_tag, mach_tag))

# done
sys.exit(0)
