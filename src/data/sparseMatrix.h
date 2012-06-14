/* ./src/data/sparseMatrix.h
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

/* Sparse matrix class - fully compressed data */
#define __sparseMATRIX_H
#include<iostream>
#include<vector>
#include<map>
#include<gsl/gsl_math.h>
//#include<gsl>  // 
using namespace std;


class sparseMatrix {

private:

  map<int, map<int,float>* > rows;

protected:

public:

  sparseMatrix();
  ~sparseMatrix();

  void insert(int,int,float);
  double at(int, int);

};
