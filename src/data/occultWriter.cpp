/* ./src/data/occultWriter.cpp
 *
 * Copyright 2012 Chris Jewell <chrism0dwk@gmail.com>
 *
 * This file is part of InFER.
 *
 * InFER is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * InFER is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with InFER.  If not, see <http://www.gnu.org/licenses/>. 
 */

// Occult read/write classes

#include "occultWriter.h"

void OccultWriter::open(const char* const filename)
{
  file.open(filename,ios::out);
  if(!file.is_open()) {
    throw ios_base::failure("OccultWriter::open failed.  Check disk space and permissions\n");
  }
}



void OccultWriter::write(vector<infection*>::const_iterator start, vector<infection*>::const_iterator end)
{
  // Writes the current occults to the file:
  // start is the beginning of the occults, end is one past the last occult

  if(!file.is_open())
    throw ios_base::failure("OccultWriter::write: Attempted write to unopen file");

  vector<infection*>::const_iterator currOccult = start;

  while(currOccult != end) {
    file << (*currOccult)->label << ":" << (*currOccult)->I << " ";
    currOccult++;
  }
  file << "\n";

}



void OccultWriter::close()
{
  file.close();
}



OccultWriter::~OccultWriter()
{
  if(file.is_open()) file.close();
}
