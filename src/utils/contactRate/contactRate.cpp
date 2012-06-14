/* ./src/utils/contactRate/contactRate.cpp
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
#include <fstream>
#include <vector>
#include <cstdlib>

#include "contactMatrix.h"

using namespace std;

int main(int argc, char *argv[]) {

  if(argc != 4) {
    cerr << "USAGE: contactRate <output file> <contact matrix> <# premises>" << endl;
    exit(-1);
  }

  char* outFilename = argv[1];
  ofstream outFile;
  int N_total = atoi(argv[3]);
  int total_possCons = (N_total * N_total) - N_total;
  float contactSum;
  vector<float> contacts(N_total,0);

  contactMat cMat;

  if(cMat.init(argv[2],N_total) != 0) {
    cout << "Cannot initialise contact matrix!" << endl;
    return(1);
  }

  contactSum = 0;
  for(int i=0;i<N_total; ++i) {
    for(int j=0; j<N_total; ++j) {
      if(i == j) continue;
      contactSum += cMat.isConn(i,j);
      contacts.at(i) += cMat.isConn(i,j);
    }
  }

  outFile.open(outFilename,ios::out);
  if(!outFile.is_open()) {
    cerr << "Could not open output file" << endl;
    exit(-1);
  }

  for(size_t i=0;i<contacts.size();++i) {
    outFile << i << "\t" << contacts.at(i) / (N_total-1) << "\n";
  }

  outFile.close();

  cout << "Contact rate: " << contactSum << " out of a possible " << total_possCons  << " connections = " << (float)contactSum * 100 / (float) total_possCons << "%" << endl;

}
