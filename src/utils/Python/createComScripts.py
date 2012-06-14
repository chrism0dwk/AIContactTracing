#!/usr/bin/python
# ./createComScripts.py
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


################################################################
# Name: createComScripts.py
# Author: C. Jewell (c)
# Created: 06/02/2008
# Purpose: Creates an array of SGE command scripts from a bunch
#            of aiMCMC conf scripts
################################################################

import sys,os,re
from string import rstrip
import re

if (len(sys.argv) != 5):
    print "Usage: createComScripts.py <conf file prefix> <from> <to> <com file prefix>"
    sys.exit(1)


## Constants
confFilePrefix = sys.argv[1]
fromNum = int(sys.argv[2])
toNum = int(sys.argv[3])
comFilePrefix = sys.argv[4]

matchEpiData = re.compile('^EpiData=') # Match the EpiData line
matchOutputFile = re.compile('^output_file=') # Match the output file line
matchObsTime = re.compile('^ObsTime=') # Match the ObsTime line
matchDataFile = re.compile('^dataFilePrefix=')
matchN = re.compile('^total_pop_size=')

## Com template:

comTemplate = """#!/bin/bash

cd $HOME/storage/aiSimulator/fmd

"""



## Loop through files


for i in range(fromNum,toNum+1):

    inputFile = None
    obsTime = None
    parmFile = None
    dataFile = None
    Ntotal = None

    confFilename = confFilePrefix+"."+str(i)+'.conf'
    confFile = open(confFilename,'r')

    for line in confFile:
        line = line.rstrip()
        
        if re.search(matchEpiData,line):
            inputFile = line.split('=')[1]
            
        if re.search(matchOutputFile,line):
            parmFile = line.split('=')[1]
            
        if re.search(matchObsTime, line):
            obsTime = line.split('=')[1]

        if re.search(matchDataFile, line):
            dataFile = line.split('=')[1]

        if re.search(matchN, line):
            Ntotal = line.split('=')[1]

    confFile.close()

    ## Check:
    if inputFile == None or parmFile == None or obsTime == None or dataFile == None or Ntotal == None:
        print "Error: Missing data in " + confFilename
        continue

    ## Write com file
    comFilename = comFilePrefix + str(i) + ".com"
    comFile = open(comFilename,'w')
    comFile.write(comTemplate)

    outFilePrefix = "$HOME/scratch/fmdPostSims/fmdPostSim." + str(i)

    comFile.write("#$ -o $HOME/scratch/fmdPostSims/fmdPostSim." + str(i) + ".stdout\n")
    comFile.write("#$ -e $HOME/scratch/fmdPostSims/fmdPostSim." + str(i) + ".stderr\n")
    comFile.write("#$ -S /bin/bash\n\n")

    comFile.write(". /etc/profile\n")
    comFile.write("module add dot\n")
    comFile.write("module add pgi\n")
    comFile.write("cd $HOME/storage/aiSimulator/fmd\n")

    comFile.write("$HOME/storage/aiSimulator/fmd/aiPosteriorSim " + outFilePrefix + " " + parmFile + " " + inputFile + " 82138 0.01 0.4 1 " + " $HOME/storage/MCMC/fmdMCMC/" + dataFile + " 0 1 " + obsTime + " 500 30000\n\n")



    comFile.close()


print "Done"


## The end
                            
