#!/usr/bin/python
# ./countNetDegree.py
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


# Script to parse contact matrix files and count node degree

import sys,os

if len(sys.argv) != 2:
  print "USAGE: countNetDegree.py <contact matrix file>"
  sys.exit(1)
  

cMatrixFilename = sys.argv[1]

# Open file
fh = open(cMatrixFilename,'r')

# Count contacts
counts = []

for line in fh:
  line = line.rstrip()
  numContacts = 0
  for char in line:
    if char == '1':
      numContacts += 1
  counts.append(numContacts)
  

# Print to stdout

for i in range(len(counts)):
  print "%i %i" % (i,counts[i])
  
