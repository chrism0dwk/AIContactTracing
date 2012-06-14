/* ./src/utils/occultFreq/occultFreq.cpp
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
#include <fstream>
#include <vector>
#include <stdexcept>
#include <map>
#include <cstdlib>

#include "occultReader.h"
#include "stlStrTok.hpp"

using namespace std;

int main(int argc, char* argv[]) {

  if(argc != 5) {
    cout << "Usage: occultFreq <output file> <known data> <occ file> <popn size>\n" << endl;
    exit(-1);
  }

  const char* const outputFilename = argv[1];
  const char* const epiFilename = argv[2];
  const char* const occultFilename = argv[3];
  int Ntotal = atoi(argv[4]);

  ifstream epiFile;
  map<size_t,double> dataRow;
  OccultReader occultReader;

  vector<size_t> knownIds;
  vector<double> occultResults(Ntotal,0);


  // Open the epidemic file and read in data
  {
    string linebuffer;

    epiFile.open(epiFilename,ios::in);
    if(!epiFile.is_open()) {
      cerr << "Cannot open epidemic file, check path and permissions\n";
      exit(-1);
    }
    
    while(!epiFile.eof()) {
      vector<string> lineTokens;

      getline(epiFile,linebuffer);
      if(linebuffer.empty()) break;
      try {
	stlStrTok(lineTokens,linebuffer," ");
      }
      catch (exception& e) {
	cerr << "Exception parsing epidemic file\n"
	     << "\tException: " << e.what() << endl;
	abort();
      }

      knownIds.push_back(atoi(lineTokens.at(0).c_str())); // 1st column contains
    }                                    // the label

  }


  // Open the occult file and read in data
  try{
    occultReader.open(occultFilename);
  }
  catch (exception& e) {
    cerr << "Exception occurred opening occult file\n"
	 << "\tException: " << e.what() << endl;
  }
  
  // Iterate over the occult posterior
  for(size_t i=0;i<occultReader.size();++i) {
    
    dataRow = occultReader.fetch(i);

    // Delete the known infections - these are NOT occults
    for(size_t k=0;k<knownIds.size();++k) {
      dataRow.erase(knownIds.at(k));
      occultResults.at(knownIds[k])++;
    }
    
    // Iterate over dataRow and mark up occults
    map<size_t,double>::iterator j = dataRow.begin();
    while(j != dataRow.end()) {
      occultResults.at(j->first)++;
      j++;
    }
    
  }
  
  
  // Divide the vector by the number of realisations
  
  for(size_t i=0;i<occultResults.size();++i) {
    occultResults.at(i) = occultResults.at(i) / occultReader.size();
  }
  
  
  // Output to file
  
  try{
    
    ofstream outFile(outputFilename,ios::out);
    for(size_t i=0;i<occultResults.size();++i) {
      outFile << i << "\t" << occultResults.at(i) << "\n";
    }
    outFile.close();
    
  }
  catch (exception& e) {
    cerr << "Exception occurred during file output\n"
	 << "\tException: " << e.what() << endl;
  }
  
  return(0);
}
