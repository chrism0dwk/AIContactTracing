#!/usr/bin/python
# ./getPosteriorMeans.py
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


import os
import sys,re
from stats import mean


if len(sys.argv) != 3:
    print "Usage: getPosteriorMeans.py <posterior dir> <output file>\n"
    sys.exit(1)


cwd = sys.argv[1]
matchParms = re.compile(".parms$")

myFiles = os.listdir(cwd)

myMeans = [[] for i in range(7)]

outFile = open(sys.argv[2],'w')

# Calculate means
for myFile in myFiles:
    if re.search(matchParms,myFile):
        posterior = [0 for i in range(7)]
        print "Opening: " + myFile
        fh = open(cwd+"/"+myFile,'r')

        N = 0
        lineCounter = 0
        for line in fh:
            toks = line.split()
            if(len(toks)>1 and lineCounter >= 30000):
                N = N+1
                for i in range(7):
                    posterior[i] = posterior[i]+float(toks[i])
            lineCounter = lineCounter + 1
            
        fh.close()
        
        for i in range(7):
            posterior[i] = posterior[i]/float(N)
            outFile.write(str(posterior[i]) + " ")

        outFile.write("\n")


outFile.close()

# end


                
