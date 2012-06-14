/* ./src/utils/contactSim/contactSim.cpp
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
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <math.h>

using namespace std;

int main(int argc, char *argv[]) {

  int N_total = atoi(argv[1]);
  float p = atof(argv[2]);
  char *outFile_name = argv[3];

  gsl_rng *rng = gsl_rng_alloc(gsl_rng_ranlux);

  ofstream outFile;

  outFile.open(outFile_name,ios::out);
  if(!outFile.is_open()) {
    cout << "Could not open file for writing!" << endl;
    return(-1);
  }

  for(int i=0; i < N_total; ++i) {
    for(int j=0; j < i; ++j) {
      if(gsl_rng_uniform(rng) < p) {
	outFile << "1";
      }
      else {
	outFile << "0";
      }
    }
    outFile << "1\n";
  }

  outFile.close();

  return(0);
}
