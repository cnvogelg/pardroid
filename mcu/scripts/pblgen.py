#!/usr/bin/env python3
# Usage:
#  pblgen.py in.bin max_size out.pbl

import os
import sys
import struct

# crc16_ccitt as implemented in libavrc
def crc16_ccitt(buf):
  crc = 0
  for d in buf:
    lo8 = crc & 0xff
    hi8 = (crc >> 8) & 0xff
    d ^= lo8
    d ^= d << 4
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

# pablo header
hdr_size = 2
max_free = max_size - hdr_size

# read input binary
with open(in_bin, "rb") as fh:
  in_data = fh.read()

# total size
n = len(in_data)
if n > max_free:
  print("TOO LARGE:", n, ">", max_free)
  sys.exit(1)

# pad image to full size
if n < max_free:
  in_data += (max_free - n) * b'\xff'

# calc checksum
check_sum = crc16_ccitt(in_data)

# create rom footer
footer_data = struct.pack("<H", check_sum)
# rom data
rom_data = in_data + footer_data

# pablo header
pbl_hdr = b'PBL1' + struct.pack(">I", max_size)

# write imaget
with open(out_pbl, "wb") as fh:
  fh.write(pbl_hdr)
  fh.write(rom_data)

out_file = os.path.basename(out_pbl)
print("%-16s  CRC16=%04x" % (out_file, check_sum))
sys.exit(0)
