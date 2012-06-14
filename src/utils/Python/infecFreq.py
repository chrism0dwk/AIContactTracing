#!/usr/bin/python
# ./infecFreq.py
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


# This script contains code to obtain the number
# of times a farm got infected in a bunch of
# simulations.


# infecs is a class to hold the farm data

class Infecs:
    def __init__(self,nFarms):
        self.farms = [0 for i in range(nFarms)]

    def addFarm(self,myFarm):
        self.farms[int(myFarm)] += 1

    def get(self,myFarm):
        return self.farms[myFarm]


# Now we begin the main program

import sys,re
from os import listdir
from string import split

if (len(sys.argv) != 4):
    print "Usage: infecFreq <no. farms> <prefix> <output file>\n" 
    sys.exit()

nFarms = int(sys.argv[1])
dirListing = listdir('./')
matchPrefix = re.compile('^'+ sys.argv[2]) # Set our search string for the sims
matchSuffix = re.compile('\.sim$') # Match the .sim suffix

farms = Infecs(nFarms) # Create an instance of the infecs class

# Loop through the files

for file in dirListing:
    if (re.search(matchSuffix,file)):
        if (re.search(matchPrefix,file)):
            print "Opening " + file + "\n"
            inFile = open(file,'r') # Open the file
            
            while 1:
                
                linestring = inFile.readline()
                if linestring == '':
                    print "Finished parsing\n"
                    break
                
                lineFrags = linestring.split() # Split the line using whitespace
                if lineFrags[2] == 'i':        # If we have an infection
                    farms.addFarm(lineFrags[1])# Add it to the farms list

            inFile.close()
                    

# Now we write out the results to the destination file

resultFile = open(sys.argv[3],'w')

for i in range(nFarms):
    writeString = str(farms.get(i)) + '\n'
    resultFile.write(writeString)
    print "Written " + str(i) + "\n"

resultFile.close()

print "Wrote file" + sys.argv[3] + "\n"
print "Num sims: " + str(len(dirListing)) + "\n"

# THE END #
    
