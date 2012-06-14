#!/usr/bin/python
# ./outputParser.py
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


import sys,re
from string import split

if (len(sys.argv) != 2):
  print "Usage: outputParser.py <stdout file>\n"
  sys.exit(1)

myRegEx = re.compile('rates')
acceptance = []


stdoutFile = open(sys.argv[1])

while 1:
  linestring = stdoutFile.readline()
  if linestring == '': 
    print "Finished parsing\n"
    break

  if(re.search(myRegEx,linestring)):
    lineToks = linestring.split()
    acceptance.append(lineToks[4])

print "c(",
for i in acceptance:
  print str(i) + ",",
print ")"

sys.exit(0)
