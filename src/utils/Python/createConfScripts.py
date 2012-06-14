#!/usr/bin/python
# ./createConfScripts.py
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


###########################################################
# Name: createConfScripts.py
# Author: C. Jewell (C)
# Created: 06/02/2008
# Purpose: Creates an array of aiMCMC configuration files
#            from a load of .ipt input files
###########################################################

import sys,re
from os import listdir
from string import atof

if (len(sys.argv) != 5):
    print "Usage: createConfScripts.py <input dir> <MCMC output prefix> <conf template> <conf prefix>\n\n"
    sys.exit()

inputDir = sys.argv[1]
outputPrefix = sys.argv[2]
templateFilename = sys.argv[3]
confPrefix = sys.argv[4]

dirListing = listdir(sys.argv[1])
matchSuffix = re.compile('\.trunc\.ipt$') # Match the .ipt suffix
matchEpiData = re.compile('^EpiData=') # Match the EpiData line
matchOutputFile = re.compile('^output_file=') # Match the output file line
matchObsTime = re.compile('^ObsTime=') # Match the ObsTime line

print "Found " + str(len(dirListing)) + " files\n"

# Loop through input files

counter = 1

for file in dirListing:
    if re.search(matchSuffix,file):
        print "Got file: " + file + "\n"
        # First read file and get the max N or R time
        inFile = open(inputDir+"/"+file,'r')
        numbers = []
        for line in inFile:
            toks = line.split()
            numbers.append(atof(toks[2]))
            numbers.append(atof(toks[3]))

        maxTime = max(numbers)
        inFile.close()
        

        # Now take the template, modify, and write out
        
        templateFile = open(templateFilename,'r')
        confFilename = confPrefix + "." + str(counter) + ".conf"
        confFile = open(confFilename,'w')

        for line in templateFile:
            if re.search(matchEpiData,line):
                confFile.write("EpiData="+inputDir+"/"+file[:-4]+"\n")

            elif re.search(matchObsTime,line):
                confFile.write("ObsTime="+str(maxTime)+"\n")

            elif re.search(matchOutputFile,line):
                confFile.write("output_file="+outputPrefix+"."+str(counter)+"\n")
            else:
                confFile.write(line)

        confFile.close();
        templateFile.close();
        counter = counter + 1
