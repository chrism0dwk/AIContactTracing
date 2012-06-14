/* ./src/utils/contactTest/contactCheck.cpp
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
#include <contactMatrix.h>
#include <sinrEpi.h>
#include <vector>
using namespace std;


int main(int argc, char *argv[]) {

  vector<int> farms;
  contactMat cMat;
  int n_total = atoi(argv[2]);
  char epiFile[50];
  int connections;
  sinrEpi myEpi;

  cMat.init(argv[1],n_total);

  //cout << "Input file:";
  //cin >> epiFile;

  //myEpi.init(n_total,epiFile);

  for(int i=0; i<n_total; ++i) {
    connections = 0;
    for(int j=i; j<n_total; ++j) {
      if(i==j) continue;
      if(cMat.isConn(i,j) ) {
	connections += 1;
      }
      cout << connections << endl;
//       else {
// 	cout << myEpi.infected[i].label << "," << myEpi.infected[j].label << "not!" << endl;
//       }
    }
  }

  return(0);

}
