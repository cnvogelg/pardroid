#!/usr/bin/env python3
# Usage:
#  pblgen.py in.bin max_size out.pbl

import os
import sys
import struct

# crc16_ccitt as implemented in libavrc
def crc16_ccitt(buf):
  crc = 0xffff
  for d in buf:
    lo8 = crc & 0xff
    hi8 = (crc >> 8) & 0xff
    d ^= lo8
    d ^= (d << 4) & 0xff
    a = d << 8 | hi8
    crc = (a ^ (d >> 4) ^ (d << 3)) & 0xffff
  return crc

# get params
if len(sys.argv) != 4:
  print("Usage: <in.bin> <max_size> <out.pbl>")
  sys.exit(1)

in_bin = sys.argv[1]
max_size = int(sys.argv[2])
out_pbl = sys.argv[3]

# pablo footer in ROM (little endian):
# ROMEND-2: crc16_ccitt (0..ROMEND-2)
# ROMEND-4: mach_tag
# ROMEND-6: version_tag
# ROMEND-8: fw_id
hdr_size = 8

# free program range
max_free = max_size - hdr_size

# read input binary
with open(in_bin, "rb") as fh:
  in_data = fh.read()

# total size of program
n = len(in_data)
if n > max_free:
  print("TOO LARGE:", n, ">", max_free)
  sys.exit(1)

# find fw info in ROM
pos = in_data.find(b'FWID')
if pos > 0:
  fw_info_fmt = ">HHH" # big endian
else:
  pos = in_data.find(b'DIWF')
  if pos > 0:
    fw_info_fmt = "<HHH" # little endian
  else:
    print("NO FWID found!!")
    sys.exit(1)
# skip magic
pos += 4
fw_id, version_tag, mach_tag = struct.unpack_from(fw_info_fmt, in_data, pos)

# pad image to full size
if n < max_free:
  rom_data = in_data + (max_free - n) * b'\xff'
else:
  rom_data = in_data

# append pablo footer without crc
pablo_footer = struct.pack(fw_info_fmt, fw_id, version_tag, mach_tag)
rom_data += pablo_footer

# calc checksum
check_sum = crc16_ccitt(rom_data)

# append crc
crc = struct.pack("<H", check_sum)
# rom data
rom_data += crc

# re-check crc
new_crc = crc16_ccitt(rom_data)
if new_crc != 0:
  print("INVALID CRC!")
  sys.exit(1)

# re-check size
if len(rom_data) != max_size:
  print("INVALID SIZE!")
  sys.exit(1)

# write imaget
with open(out_pbl, "wb") as fh:
  fh.write(rom_data)

# write message
out_file = os.path.basename(out_pbl)
print("%-16s  SIZE=%06x  CRC16=%04x  FW=%04x  VER=%04x  MACH=%04x" % \
  (out_file, max_size, check_sum, fw_id, version_tag, mach_tag))

# done
sys.exit(0)
