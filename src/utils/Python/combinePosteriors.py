#!/usr/bin/python
# ./combinePosteriors.py
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


#######################################################################
# Name: combinePosteriors                                             #
# Author: C Jewell (c)                                                #
# Created: 21/02/08                                                   #
# Purpose: Takes a group of .parms files, and combines the posteriors #
#            into separate files                                      #
#######################################################################


import os,sys
import re
from string import split

if(len(sys.argv) != 4):
    print "Usage: combinePosteriors.py <input dir> <file prefix> <output prefix>"
    sys.exit(1)

# Constants
cwd = sys.argv[1]
outPrefix = sys.argv[3]
numParms = 7
parmMatch = re.compile(".parms$")
prefMatchStr = "^" + sys.argv[2]
prefMatch = re.compile(prefMatchStr)

# Read files
myFiles = os.listdir(cwd)

# Create output files
fileHandles = [0 for i in range(numParms)]
for i in range(numParms):
    fileHandles[i] = open(outPrefix + "." + str(i) + ".txt",'w')

# Loop over .parms files and extract parameters
for filename in myFiles:

    if re.search(parmMatch, filename) and re.search(prefMatch,filename):

        print "Reading filename: " + cwd + "/" + filename
        
        inFile = open(cwd+"/"+filename,'r')
        for line in inFile:
            toks = line.split();
            if(len(toks) > 1): # Make sure we didn't get an empty line
                for i in range(numParms):
                    fileHandles[i].write(str(toks[i])+"\n")
        inFile.close()


# Close output files
for fh in fileHandles:
    fh.close()

#########################################
# Use RPy to create graphs - future!    #
#########################################
