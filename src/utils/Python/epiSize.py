#!/usr/bin/python
# ./epiSize.py
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


# This script contains code to obtain the size of an epidemic from
# a sim file.



# Now we begin the main program

import sys,re
from os import listdir
from string import split

if (len(sys.argv) != 2):
    print "Usage: epiSize.py <prefix> \n" 
    sys.exit()

dirListing = listdir('./')
matchPrefix = re.compile('^'+ sys.argv[1]) # Set our search string for the sims
matchSuffix = re.compile('\.sim$') # Match the .sim suffix

# Loop through the files

for file in dirListing:
    if (re.search(matchSuffix,file)):
        if (re.search(matchPrefix,file)):
            inFile = open(file,'r') # Open the file
            epidemic = [] # List of tuples
            
            while 1:
                
                linestring = inFile.readline()
                if linestring == '':
                    break
                
                lineFrags = linestring.split() # Split the line using whitespace
                epidemic.append(lineFrags)

            inFile.close()

            epiLine = epidemic.pop()
            print str(epiLine[6])
                    

# THE END #
    
