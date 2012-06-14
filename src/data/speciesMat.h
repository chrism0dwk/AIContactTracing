/* ./src/data/speciesMat.h
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

// This class sets up a storage matrix for species
// It takes a matrix of 1's and 0's with only 1 or 0 occurances
// of a 1 in any row.

#ifndef _INCLUDE_SPECIESMAT_H
#define _INCLUDE_SPECIESMAT_H

#include <iostream>
#include <fstream>
#include <string>

using namespace std;
 
class SpeciesMatrix {
 private:
  bool** speciesMat;
  size_t nPremises,nSpecies;
  int rv;
  bool isInit;

 public:
  SpeciesMatrix();
  ~SpeciesMatrix();
  int initialize(const char[],const size_t, const size_t);
  double at(const size_t,const size_t);
};

#endif
