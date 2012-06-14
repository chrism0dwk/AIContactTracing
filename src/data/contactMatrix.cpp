/* ./src/data/contactMatrix.cpp
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

#ifndef INCLUDE_CONTACTMATRIX_H
#include "contactMatrix.h"
#endif

#define SET_BIT(x,y) *(*(contact_bitmap+y)+x/8) |= (0x80 >> ((x)%8))
#define GET_BIT(y,x) (*(*(contact_bitmap+y)+x/8) & (0x80 >> ((x)%8)))

contactMat::contactMat() {
}

contactMat::~contactMat() {
  for(int i=0; i < N_total; ++i) {
    delete[] *(contact_bitmap+i);
  }
  delete[] contact_bitmap;
}



int contactMat::init(const char *filename, int myN_total) {

  int i,j;
  N_total = myN_total;

  cerr << "Reading Contact Matrix '" << filename << "'" << endl;

  contact_bitmap = new char*[N_total]; // Create an array of pointers

  #pragma omp parallel for default(shared) private(i,j) schedule(static)
  for(i=0; i < N_total; ++i) {
     *(contact_bitmap+i) = new char[N_total/8 + 1];
     for(j=0; j < (N_total/8 +1); ++j) {
       *(*(contact_bitmap+i)+j) = 0x00;
     }
  }


  ifstream inFile;
  char *line = new char[N_total+1];

  inFile.open(filename,ios::in);
  if(!inFile.is_open()) {
    cerr << "Error opening contact matrix file '" << filename << "'" << endl;
    return(-1);
  }

  for(int i=0; i < N_total; ++i) {
    if(inFile.eof()) {
      cerr << "Premature EOF in contactMat::fileGen" << endl;
      return(-1);
    }

    inFile.getline(line,N_total+1);
    if(line[0] == '\0') {
      cerr << "Empty line encountered!" << endl;
      return(-1);
    }

    for(int j=0; j <= i; ++j) {
      if(line[j] == '1') {
	SET_BIT(j,i);
        SET_BIT(i,j);
      }
    }
  }

  inFile.close();

  return(0);
}



float contactMat::isConn(int x, int y) {
  if(GET_BIT(x,y) !=0) return 1.0;
  else return 0.0;
}

