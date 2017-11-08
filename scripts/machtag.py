#!/usr/bin/env python3
# machtag.py

import sys
import os
import argparse
import re

# ----- definition of machtags -----
machtags = {
    "avr" : {
        "id" : 1,
        "cpus" : {
            "atmega328" : 1,
            "atmega32" : 2,
            "atmega32u4" : 3
        },
        "machs" : {
            "ardunano" : 1,
            "avrnetio" : 2,
            "teensy20" : 3
        }
    },
    "mk20" : {
        "id" : 2,
        "cpus" : {
            "mk20dx256" : 1
        },
        "machs" : {
            "teensy32" : 1
        }
    }
}


def lookup(arch, cpu, mach, extra=0):
  if arch not in machtags:
    raise ValueError("invalid arch: " + arch)
  d = machtags[arch]
  arch_id = d['id']
  cpus = d['cpus']
  machs = d['machs']
  if cpu not in cpus:
    raise ValueError("invalid cpu: " + cpu)
  cpu_id = cpus[cpu]
  if mach not in machs:
    raise ValueError("invalid mach: " + mach)
  mach_id = machs[mach]
  return (arch_id, cpu_id, mach_id, extra)


def print_hex_value(mt):
  tag = mt[0] << 12 | mt[1] << 8 | mt[2] << 4 | mt[3]
  print("0x%04x" % tag)


def print_define(arch, cpu, mach, extra):
  a = arch.upper()
  c = cpu.upper()
  m = mach.upper()
  if extra == 0:
    print("MACHTAG_{}_{}_{}".format(a,c,m))
  else:
    print("MACHTAG_{}_{}_{}_{}".format(a,c,m, extra))


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('arch', help="cpu architecture")
  parser.add_argument('cpu', help="cpu name")
  parser.add_argument('mach', help="machine name")
  parser.add_argument('extra', nargs='?', default=0, type=int, help="extra tag")
  parser.add_argument('-v', '--print-value', action='store_true', default=False, help="print build tag")
  parser.add_argument('-d', '--print-define', action='store_true', default=False, help="print build tag")
  args = parser.parse_args()

  mt = lookup(args.arch, args.cpu, args.mach, args.extra)
  if args.print_value:
    print_hex_value(mt)
  if args.print_define:
    print_define(args.arch, args.cpu, args.mach, args.extra)

if __name__ == '__main__':
  sys.exit(main())
