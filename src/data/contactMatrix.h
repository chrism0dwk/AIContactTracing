/* ./src/data/contactMatrix.h
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

/* contactMatrix takes the *lower* triangle of a contact matrix and stores it as a bitmap */

#ifndef INCLUDE_CONTACTMATRIX_H
#define INCLUDE_CONTACTMATRIX_H

#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;

class contactMat {
 private:
  int N_total;
  char **contact_bitmap;
 public:

  contactMat();
  ~contactMat();
  
  int init(const char*,int);
  float isConn(int,int);
};

#endif
