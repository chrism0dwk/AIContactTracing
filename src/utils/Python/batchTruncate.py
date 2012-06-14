#!/usr/bin/python
# ./batchTruncate.py
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

from os import listdir
import re
from string import split
from truncateSim import truncateSim


if (len(sys.argv) != 2):
    print "Usage: batchTruncate.py <batch dir>"

cwd = sys.argv[1]
matchIpt = re.compile(".ipt$")
myFiles = listdir(cwd)

for myFile in myFiles:
    if re.search(matchIpt,myFile):
        print "Truncating " + myFile

        prefix = cwd+"/"+myFile[:-4]

        fh = open(cwd+"/"+myFile,'r')

        toks = []
        for line in fh:
            toks = line.split()

        fh.close()
        truncTime = float(toks[3])/2

        myEpi = truncateSim(prefix+".ipt",prefix+".contact.xml",truncTime)
        myEpi.writeIpt(prefix+".trunc.ipt")
        myEpi.writeCTData(prefix+".trunc.contact.xml")


        
