/* ./src/data/sparseMatrix.cpp
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

/* Class methods for sparseMatrix class - see header for declarations */

#ifndef __sparseMATRIX_H
#include "sparseMatrix.h"
#endif

sparseMatrix::sparseMatrix() {

}

sparseMatrix::~sparseMatrix() {

}

void sparseMatrix::insert(int row, int col, float value) {

  if(rows.find(row) == rows.end()) {
    rows[row] = new map<int,float>;
  }

  (*rows[row])[col] = value;

}

double sparseMatrix::at(int row, int col) {

  map<int, map<int,float>* >::iterator row_ptr;
  map<int,float>::iterator col_ptr;
  
  row_ptr = rows.find(row);

  if(row_ptr == rows.end()) {
    return(GSL_POSINF);
  }

  else {
    col_ptr = row_ptr->second->find(col);
    
    if(col_ptr == row_ptr->second->end()) {
      return(GSL_POSINF);
    }

    else{
      return(col_ptr->second);
    }
  }
  
}
