#!/usr/bin/python
# ./occultSurv.py
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


# Created on: 2 Feb 2010
# Author: Chris Jewell, chris.jewell@warwick.ac.uk


import os,sys
from optparse import OptionParser
from string import split


class Occult:
    def __init__(self,label,occProb):
        self.label = int(label)
        self.occProb = float(occProb)
    def __cmp__(self,other):
        if self.occProb < other.occProb:
            return -1
        elif self.occProb == other.occProb:
            return 0
        else:
            return 1
        
class Infective:
    def __init__(self,I,N,R):
        self.I = float(I)
        self.N = float(N)
        self.R = float(R)
    def isKnownAt(self,time):
        if self.N < time:
            return True
        else:
           return False
    def __str__(self):
        return str(self.I)+" "+str(self.N)+" "+str(self.R)
    def __repr__(self):
        return str(self.I)+" "+str(self.N)+" "+str(self.R)


usage = "USAGE: %prog [options] <input prefix> <occFilePrefix> <output prefix>"
optparser = OptionParser(usage=usage)
optparser.add_option("-n", "--num-occults", dest="numOccults",
                     help="Number of occults to visit", default=10,type="int")
optparser.add_option("-t", "--obs-time", dest="obsTime",
                     help="Epidemic observation time", default=1e10,type="float")

(options,args) = optparser.parse_args()
if len(args) != 3:
    print optparser.print_usage()
    sys.exit(1)

# First get top n occults
occFile = open(args[1],'r')
occults = []
for line in occFile:
    toks = line.split()
    if float(toks[1]) < 1.0: # Ignore occProb == 1 as these are known infections
        occults.append(Occult(toks[0],toks[1]))
occFile.close()

occults = sorted(occults,reverse=True)
occults = occults[0:options.numOccults]

for i in occults:
    print str(i.label) + ": " + str(i.occProb)


# Read in ipt file
iptFile = open(args[0]+".uncensored.ipt",'r')
infectives = {}
for line in iptFile:
    toks = line.split(" ")
    infectives[int(toks[0])] = Infective(float(toks[1]),float(toks[2]),float(toks[3]))
iptFile.close()

# Get not infected list
notInfected = {}
niFile = open(args[0]+".ni")
for line in niFile:
    toks = line.split(" ")
    notInfected[int(toks[0])] = float(toks[1])
niFile.close()


# Get dc list
dcList = {}
dcFile = open(args[0]+".dc")
for line in dcFile:
    toks = line.split()
    dcList[int(toks[0])] = Infective(0,float(toks[1]),float(toks[2]))
dcFile.close()


# Look for occults in the (uncensored) ipt file
for occult in occults:
    if occult.label in infectives:
        if occult.label in dcList:
            raise Exception("Duplicate in DC list: "+str(occult.label))
        else:
            dcList[occult.label] = Infective(0,options.obsTime,options.obsTime)
    else:
        notInfected[occult.label] = options.obsTime


# Output .ni and .dc files
ofile = open(args[2]+".dc",'w')
for label in dcList:
    ofile.write(str(label) + " " + str(dcList[label].N) + " " + str(dcList[label].R) + "\n")
ofile.close()

ofile = open(args[2]+".ni",'w')
for label in notInfected.keys():
    ofile.write(str(label) + " " + str(notInfected[label]) + "\n")
ofile.close()
