#!/usr/bin/python
# ./getNumInfecs.py
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


## Counts number of new infections in a 2 week period starting at the beginning of the epidemic

import sys
from os import listdir
from string import split,rstrip
import re

if len(sys.argv) != 4:
    print "Usage getNumInfecs.py <sim directory> <time> <output file>"
    sys.exit(1)
    

# Constants

simDir = sys.argv[1]
time = float(sys.argv[2])
outputFilename = sys.argv[3]
allSims = listdir(simDir)

# Set up dict to hold epi size
epiSizes = {41 : []}

for i in range(42,72):
    epiSizes[i] = []


# Loop through files
for sim in allSims:

    fnCrumbs = sim.split('.')

    if fnCrumbs[-1] != 'sim':
        continue

    myDay = int(fnCrumbs[1])

    # Read file into memory
    myEpidemic = []

    mySim = open(simDir+'/'+sim,'r')
    for line in mySim:
        line = line.rstrip()
        if len(line) <=1:
            continue
        crumbs = line.split()
        myEpidemic.append(crumbs)
    mySim.close()

    # Next if no lines in the epidemic
    if len(myEpidemic) == 0:
        continue

    # Start at the beginning, read number of infections
    startTime = float(myEpidemic[0][0])
    endTime = startTime + 7
    eventCount = 0

    for crumbs in myEpidemic:

        if float(crumbs[0]) > endTime:
            break

        if crumbs[2] == 'i':
            eventCount = eventCount + 1

    epiSizes[myDay].append(eventCount)



## Write sizes to CSV file

outputFile = open(outputFilename,'w')

for key in epiSizes.keys():
    outputFile.write(str(key)+" ")

    vals = epiSizes[key]
    for i in range(3940):
        if(i<len(vals)):
            outputFile.write(str(vals[i])+" ")
        else:
            outputFile.write('NA ')

    outputFile.write("\n")

outputFile.close()
