#!/usr/bin/env python3
# Usage:
#  avr-size -A <bin> | checksize.py <max_rom> <max_ram>

import os
import sys

# get max size for ROM and RAM from command line
if len(sys.argv) != 4:
  print("Usage: <max_rom> <max_ram> <name>")
  sys.exit(1)

max_rom = int(sys.argv[1])
max_ram = int(sys.argv[2])
name = sys.argv[3]

# parse avr-size -A output
res = {}
for line in sys.stdin:
  line = line.strip()
  elem = line.split()
  if len(elem) >= 2:
    tag = elem[0]
    if tag in ('.text', '.data', '.bss'):
      res[tag] = int(elem[1])

if len(res) < 2:
  print("Missing sections?")
  sys.exit(2)

# calc size
rom_size = res['.text']
ram_size = res['.data']
if '.bss' in res:
  ram_size += res['.bss']

# check
ret = 0
if rom_size <= max_rom:
  rom_tag = "ok"
else:
  rom_tag = "BIG"
  ret = 1
if ram_size <= max_ram:
  ram_tag = "ok"
else:
  ram_tag = "BIG"
  ret = 1

print("%-16s  ROM: %3s    got: %5d  max: %5d" % (name, rom_tag, rom_size, max_rom))
print("%-16s  RAM: %3s    got: %5d  max: %5d" % (name, ram_tag, ram_size, max_ram))

sys.exit(ret)
