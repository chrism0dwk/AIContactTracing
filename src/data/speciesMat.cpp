/* ./src/data/speciesMat.cpp
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

// Implementation of species storage class
#include "speciesMat.h"

SpeciesMatrix::SpeciesMatrix() :
  nPremises(0),
  nSpecies(0),
  isInit(0)
{
  // Constructor takes no args
}



SpeciesMatrix::~SpeciesMatrix()
{
  // Destructor cleans up nicely.
  if(isInit == 1) {
  for(size_t i=0; i < nSpecies; ++i) {
    delete[] speciesMat[i];
  }

  delete[] speciesMat;
  }
}

int SpeciesMatrix::initialize(const char filename[],const size_t n, const size_t p)
{
  // Constructor takes args filename (name of matrix file),
  // n (no of premises), and s (no of species)

  ifstream inputFile;
  string line;

  // First set the values:
  nPremises = n;
  nSpecies = p;

  // Create an array of pointers to array
  //  - this is our "rows"
  speciesMat = new bool*[nPremises];

  for(size_t row=0; row < nPremises; ++row) {
    speciesMat[row] = new bool[nSpecies];
  }

  // Open our input file and read in the contents
  inputFile.open(filename,ios::in);
  if(!inputFile.is_open()) {
    cerr << "Unable to open file " << filename << endl;
    return(-1);
  }

  for(size_t row=0; row < nPremises; ++row) {

    getline(inputFile,line);
    if (line.size() != nSpecies) {
      cerr << "Invalid file format! Line width = " << line.size() << endl;
      return(-2);
    }

    for(size_t col=0; col < nSpecies; ++col) {
      if(line.at(col) == '1') {
	speciesMat[row][col] = 1;
      }
      else {
	speciesMat[row][col] = 0;
      }
    }
  }

  isInit = 1;
  return 0;
}



double SpeciesMatrix::at(const size_t premises, const size_t species)
{
  // Returns an entry in the species matrix
  if(premises < nPremises && species < nSpecies) {
    if(speciesMat[premises][species] == 0) {
    return 0.0;
    }
    else {
      return 1.0;
    }
  }
  else {
    cout << premises << "," << species << " not in range [" << nPremises << "," << nSpecies << "]" << endl;
    throw "Range error";
  }
}
