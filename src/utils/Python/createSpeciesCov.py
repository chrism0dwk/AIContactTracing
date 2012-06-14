#!/usr/bin/python
# ./createSpeciesCov.py
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


import sys,pg

if len(sys.argv) < 1:
  print "Usage: createSpeciesCov.py <output filename>\n"
  sys.exit(1)

outputFilename = sys.argv[1]

dbName = "gbpr"
dbHost = "localhost"
dbPort = 3333
dbUser = "jewellc"
dbPass = "lums2005"

qryGetData = "SELECT species FROM network_cleaned_tbl ORDER BY label"

try:
  myDb = pg.connect(dbName,dbHost,dbPort,'','',dbUser,dbPass)
except pg.InternalError:
  print 'Connection error!'
  sys.exit(1)

qryResults = myDb.query(qryGetData)
print 'Parsing ' + str(qryResults.ntuples()) + ' records'

outputFile = open(outputFilename,'w')

for row in qryResults.dictresult():

  speciesRow = {'Broilers': 0,
 		'Clayers': 0,
 		'Dlayers': 0,
 		'Duck': 0,
 		'Glayers': 0,
 		'Goose': 0,
 		'Partridge': 0,
 		'Pheasant': 0,
 		'Qlayers': 0,
 		'Turkey': 0}

  speciesRow[row['species']] = 1
 
  #We don't output the broilers since this is implicit
  #outputFile.write(str(speciesRow['Broilers'])) 
  #
  outputFile.write(str(speciesRow['Clayers']))
  outputFile.write(str(speciesRow['Dlayers']))
  outputFile.write(str(speciesRow['Duck']))
  outputFile.write(str(speciesRow['Glayers']))
  outputFile.write(str(speciesRow['Goose']))
  outputFile.write(str(speciesRow['Partridge']))
  outputFile.write(str(speciesRow['Pheasant']))
  outputFile.write(str(speciesRow['Qlayers']))
  outputFile.write(str(speciesRow['Turkey']))
  
  outputFile.write('\n')

outputFile.close()
myDb.close()

sys.exit(0)
