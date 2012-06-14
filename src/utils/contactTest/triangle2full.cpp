/* ./src/utils/contactTest/triangle2full.cpp
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

#include <iostream>
#include <cstdlib>

#include <contactMatrix.h>
using namespace std;


int main(int argc, char *argv[]) {

  contactMat cMat;
  int n_total = atoi(argv[2]);

  cMat.init(argv[1],n_total);

  for(int i=0; i < n_total; ++i) {
    for(int j=0; j < n_total; ++j) {
      cout << cMat.isConn(i,j) << " ";
    }
    cout << "\n";
  }

  return(0);

}
