/* ./src/utils/I1_Freq/I1Freq.cpp
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

// I1Freq extracts the posterior distribution of I1

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <map>
#include <gsl/gsl_math.h>

#include "occultReader.h"

using namespace std;

int main(int argc, char* argv[]) {

  if(argc != 4) {
    cout << "Usage: I1Freq <output file> <occ file> <popn size>\n" << endl;
    exit(-1);
  }

  int Ntotal = atoi(argv[3]);

  vector<double> I1Freqs(Ntotal,0);
  map<size_t,double> dataRow;  
  OccultReader occultReader;

    // Open the occult file and read in data
    try{
      occultReader.open(argv[2]);
    }
    catch (exception& e) {
      cerr << "Exception occurred opening occult file\n"
	   << "\tException: " << e.what() << endl;
    }

    for(size_t i=0;i<occultReader.size();++i) {
      
      dataRow = occultReader.fetch(i);

      // Iterate over dataRow to find I1
      map<size_t,double>::iterator j = dataRow.begin();
      size_t I1;
      double I1t = GSL_POSINF;

      while(j != dataRow.end()) {
	
	if(j->second < I1t) {
	  I1t = j->second;
	  I1 = j->first;
	}
	j++;
      }

      I1Freqs[I1]++; // Increment frequency
    }


    // Divide the frequency vector by the number of realisations

    for(size_t i=0;i<I1Freqs.size();++i) {
      I1Freqs.at(i) = I1Freqs.at(i) / occultReader.size();
    }


    // Output to file

    try{

      ofstream outFile(argv[1],ios::out);
      for(size_t i=0;i<I1Freqs.size();++i) {
	outFile << i << "\t" << I1Freqs.at(i) << "\n";
      }
      outFile.close();

    }
    catch (exception& e) {
      cerr << "Exception occurred during file output\n"
	   << "\tException: " << e.what() << endl;
    }

    return(0);
}
