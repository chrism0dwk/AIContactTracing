#!/usr/bin/python
# ./comparePosterior.py
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


###############################################################
# Name: comparePosterior.py
# Author: C Jewell (c)
# Created: 11/02/2008
# Purpose: t-Tests whether two marginal posteriors from aiMCMC
#            output files are significantly different.
###############################################################

import sys,os
from stats import ttest_ind
from string import split

burnIn = 30000

if len(sys.argv) != 4:
    print "Usage: comparePosterior.py <file 1> <file 2> <col number>\n"
    sys.exit(1)



fileNameA = sys.argv[1]
fileNameB = sys.argv[2]
colNum = int(sys.argv[3])

posteriorA = []
posteriorB = []


# Read first file
fileHandle = open(fileNameA,'r')

for line in fileHandle:
    toks = line.split()
    posteriorA.append(float(toks[colNum]))

fileHandle.close()


# Read second file
fileHandle = open(fileNameB,'r')
for line in fileHandle:
    toks = line.split()
    posteriorB.append(float(toks[colNum]))

fileHandle.close()



# Perform t-test

ttest_ind(posteriorA[burnIn:],posteriorB[burnIn:],printit=1,name1="Posterior1",name2="Posterior2")
