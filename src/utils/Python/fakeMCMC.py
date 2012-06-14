#!/usr/bin/python
# ./fakeMCMC.py
#
# Copyright 2012 Chris Jewell <chrism0dwk@gmail.com>
#
# This file is part of InFER.
#
# InFER is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# InFER is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with InFER.  If not, see <http://www.gnu.org/licenses/>. 


import os,sys
from string import split,rstrip

# USAGE
if len(sys.argv) != 3:
  print "USAGE: fakeMCMC.py <.ipt file> <fake occult filename>"
  sys.exit()

# Set up constants
iptFilename = sys.argv[1]
occFilename = sys.argv[2]

# Read in .ipt file
iptFile = open(iptFilename,'r')
occFile = open(occFilename,'w')

for line in iptFile:
  line = line.rstrip()
  if line == '':
    break
  fields = line.split()
  label = fields[0]
  I = fields[1]
  occFile.write(label + ":" + I + " ")

occFile.write("\n")

iptFile.close()
occFile.close()

print "Wrote " + occFilename

sys.exit(0)
