#!/usr/bin/python
# ./truncateSim.py
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


######################################################################
# Name: truncateSim.py                                               #
# Author: C. Jewell                                                  #
# Created: 04/02/2008                                                #
# Purpose: Truncates an aiSimulator simulation with contact tracing  #
######################################################################

from sys import path
path.append('/home/hpc/55/jewellc/storage/4Suite/lib/python2.5')
from string import atoi,atof
from Ft.Xml import EMPTY_NAMESPACE
from Ft.Xml.InputSource import DefaultFactory
from Ft.Xml.Domlette import NonvalidatingReader, PrettyPrint
from Ft.Lib import Uri

class Infective:
    def __init__(self,label,I,N,R):
        self.label = label
        self.I = I
        self.N = N
        self.R = R

    def __str__(self):
        return str(self.label) + " " + str(self.I) + " " + str(self.N) + " " + str(self.R)

class truncateSim:
    """Holds a truncated version of epidemic data.
    iptFile is the .ipt input file, ctFile is the contact tracing
    XML file, and truncTime is the required truncation time"""

    def __init__(self,iptFile,ctFile,truncTime):
        # Read epidemic and truncate to truncTime
        self.infectives = []
        self.labels = []
        epiFile = open(iptFile,'r')
        for line in epiFile:
            toks = line.split()
            label = atoi(toks[0])
            I = atof(toks[1])
            N = atof(toks[2])
            R = atof(toks[3])
            if N <= truncTime: # Take individuals who have been notified by truncTime
                if R > truncTime: # If R > truncTime, set R = truncTime
                    R = truncTime
                self.infectives.append(Infective(label,I,N,R))
                self.labels.append(label)
        epiFile.close()

                
        # Read in XML
        conFile = Uri.OsPathToUri(ctFile)
        xmlSrc = DefaultFactory.fromUri(conFile,stripElements=[(EMPTY_NAMESPACE,'*',1)])
        self.doc = NonvalidatingReader.parse(xmlSrc)

        # Remove from the contact DOM any contact info
        #   for individuals that are not present in labels
        self.labels = set(self.labels)
        for contact in self.doc.documentElement.xpath(u'tc:contact',explicitNss={u'tc':u'tracedcontacts'}):
            contactLabel = atoi(contact.getAttributeNS(None,u'id'))
            if contactLabel not in self.labels:
                self.doc.documentElement.removeChild(contact)


    def writeIpt(self,outputIpt):
        # Write out the truncated epidemic
        epiFile = open(outputIpt,'w')
        for infection in self.infectives:
            epiFile.write(str(infection) + "\n")
        epiFile.close()
                            

    def writeCTData(self,outputCT):
        conFile = open(outputCT,'w')
        PrettyPrint(self.doc,conFile)
        conFile.close()


# If called on our own

if __name__=="__main__":

    import sys

    if len(sys.argv) != 4:
        print "Usage: truncateSim.py <input sim prefix> <trunc time> <output sim prefix>\n"
        sys.exit(1)


    inputFile = sys.argv[1]
    truncTime = atof(sys.argv[2])
    outputFile = sys.argv[3]

    # Get data in
    myEpidemic = truncateSim(inputFile+".ipt",inputFile+".contact.xml",truncTime)

    # Write data out
    myEpidemic.writeIpt(outputFile+".ipt")
    myEpidemic.writeCTData(outputFile+".contact.xml")

    # That's all folks!
    print "Created truncated sim: " + outputFile + "\n"

    sys.exit(0)
