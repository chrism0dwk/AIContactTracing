/* ./src/data/occultReader.h
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

// occult.h contains the declarations for occultWriter and occultReader //
// classes that save information on the state of occult infections.     //

#ifndef INCLUDE_OCCULTREADER_H
#define INCLUDE_OCCULTREADER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <map>


using namespace std;

class OccultReader
{
 private:
  ifstream file;
  size_t rowPos;
  size_t posteriorSize;
  streampos startPos;
  void rewind();
  
 public:
  typedef map<size_t, double> OccultMap;

   OccultReader();
  ~OccultReader();
  void open(const char* const,const int skip=1);
  OccultMap fetch(const int&);
  size_t size();
  void close();
};

#endif
