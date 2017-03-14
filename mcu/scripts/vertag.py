#!/usr/bin/env python3
#
# vertag.py <version_major> <version_minor>
#
# generate version word

import sys

if len(sys.argv) != 3:
  print("Usage: <version_major> <version_minor>")
  sys.exit(1)

ver_maj = int(sys.argv[1])
ver_min = int(sys.argv[2])

ver_tag = ver_maj << 8 | ver_min
print("0x%04x" % ver_tag)
