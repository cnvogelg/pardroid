#!/usr/bin/env python3
#
# machtag.py <arch_id> <mcu_id> <mach_id>
#
# generate machtag word

import sys

if len(sys.argv) != 4:
  print("Usage: <arch_id> <mcu_id> <mach_id>")
  sys.exit(1)

arch_id = int(sys.argv[1])
mcu_id = int(sys.argv[2])
mach_id = int(sys.argv[3])

mach_tag = arch_id << 12 | mcu_id << 8 | mach_id
print("0x%04x" % mach_tag)
