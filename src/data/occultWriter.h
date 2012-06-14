/* ./src/data/occultWriter.h
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

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <exception>
#include <vector>
#include <string>
#include <map>

#include "sinrEpi.h"



using namespace std;

class OccultWriter
{
 private:
  ofstream file;

 public:

  ~OccultWriter();
  void open(const char* const);
  void close();
  void write(vector<infection*>::const_iterator, vector<infection*>::const_iterator);
};
