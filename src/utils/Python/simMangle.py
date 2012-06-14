#!/usr/bin/python
# ./simMangle.py
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


import sys

if len(sys.argv) < 3:
  print "Usage: simMangle <source> <dest>\n"
  exit

sourceFile = sys.argv[1]
destFile = sys.argv[2]
time = []
label = []
event = []

inFile = open(sourceFile,'r') # Open the source file

# Now go through all the lines of the source file
#linestring = inFile.readline()
while 1:
  linestring = inFile.readline()
  if linestring == '':
    print "End\n";
    break
  #linestring = linestring[:-2] # Remove the \n
  print linestring
  a,b,c,d,e,f,g = linestring.split(' ')
  time.append(a)
  label.append(b)
  event.append(c)

inFile.close()


outFile = open(destFile,'w') # Open the destination file

# Now we have the file in memory, so we begin at the top of the label vector

while len(label) > 0:

  curr_label = label[0]
  curr_I = time[0]
  curr_N = 0
  curr_R = 0
  k = 1
  while k < len(label):
    if curr_label == label[k]:
        if event[k] == 'n':
          curr_N = time[k]
          time.pop(k)
          event.pop(k)
          label.pop(k)
          print "Popped N label "+str(curr_label)
        elif event[k] == 'r':
          curr_R = time[k]
          time.pop(k)
          event.pop(k)
          label.pop(k)
          print "Popped R label "+str(curr_label)
        else: 
          print "Panic!!!!!\n"
          exit
        
    else:
      k = k + 1
  out_line = str(label[0]) + ' ' + str(curr_I) + ' ' + str(curr_N) + ' ' + str(curr_R) + '\n'
  outFile.write(out_line)
  label.pop(0)
  time.pop(0)
  event.pop(0)

outFile.close()

print "Finished\n"
