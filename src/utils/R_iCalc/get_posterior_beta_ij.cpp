/* ./src/utils/R_iCalc/get_posterior_beta_ij.cpp
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
/***************************************************************************
 *   Copyright (C) 2009 by Chris Jewell                                    *
 *   chris.jewell@warwick.ac.uk                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/* this is a program to create samples of beta_ij for any ij */

#include <iostream>
#include <ostream>
#include <vector>
#include <math.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_statistics.h>

#include "mpi.h"

/* Epidemic class */
#include "epiCovars.h"
#include "speciesMat.h"

using namespace std;

///////////// Class Definitions /////////////////

class Posterior {
  // This class sets up a 2D array to hold the posteriors
  // See below for implementation

private:
  vector< vector<double> > posteriors;
  ifstream mcmcFile;

public:
  Posterior();
  ~Posterior();
  int initialize(const char*);
  int fetch(vector<double>&, const int&);
  size_t numIterations();
};



class EpiMath {

private:

public:

  EpiMath(EpiCovars&);
  ~EpiMath();
  inline double beta_ij(const int&, const int&, const vector<double>&);

  EpiCovars& epidata;

};








////////// Functions ////////////


int main (int argc, char* argv[]) {

  MPI_Init(&argc, &argv);

  if (argc != 8) {
    cout << "USAGE: get_posterior_beta_ij <MCMC output> <burnin> <skip> <distances> <contact prefix> <population size> <out file prefix> " << endl;
    return -1;
  }
  
  
  // Declarations //
  
  const char* const mcmcOutput = argv[1];
  const int N = atoi(argv[6]);
  const int burnIn = atoi(argv[2]);
  const int thinBy = atoi(argv[3]);
  const char* const distanceFile = argv[4];
  const char* const contactPrefix = argv[5];
  const char* const outputPrefix = argv[7];
  vector<double> parms;
  ofstream outputFile;
  char outputFileName[50];
  int myId, mpiWorldSize;
  int bandSize,startIndex,endIndex;
  double* postArray;

  MPI_Comm_rank(MPI_COMM_WORLD, &myId);
  MPI_Comm_size(MPI_COMM_WORLD, &mpiWorldSize);

  EpiCovars covars;  // Class to hold our covariates
  Posterior posteriors;
  EpiMath mathHandler(covars); // Sets up the maths handler


  // Initialise the covariates //
  
  covars.init(N,contactPrefix,distanceFile);


  // Initialise the posterior object //

  if(posteriors.initialize(mcmcOutput) != 0) {
    cerr << "Error initialising posteriors!\n";
    std::abort();
  }
 

  // Calculate the bandsize to distribute the farms
  // among the MPI nodes
  bandSize = N / mpiWorldSize;
  startIndex = myId * bandSize;
  if(myId != mpiWorldSize - 1)
    endIndex = startIndex + bandSize - 1;
  else
    endIndex = N - 1;

  // Open the output file for this node
  sprintf(outputFileName, "%s_%02i.txt",outputPrefix,myId);
  outputFile.open(outputFileName);
  if(!outputFile.is_open()) {
    cerr << "Failed to open file " << outputFileName << endl;
    std::abort();
  }

  // We calculate a beta_ij for each farm and add it to the current entry

  int dataSize = (posteriors.numIterations() - burnIn) / thinBy;
  int arrayIndex;
  postArray = new double[dataSize];

  cout << "Beginning calc loop" << endl;

  for (int i=startIndex; i<=endIndex; ++i) {

    arrayIndex = 0;
    for (int parmSet=burnIn; parmSet<posteriors.numIterations()-1; parmSet += thinBy) {

      posteriors.fetch(parms,parmSet);
 
      double sum = 0.0;
      for (int j=0; j<N; j++) {
	if (i == j) continue;
	sum = sum + mathHandler.beta_ij(i, j, parms);
      }	
      sum = sum / parms.at(16);
      postArray[arrayIndex] = sum;
      ++arrayIndex;
    }
    
    // gsl_sort(postArray,1,dataSize);

//     // Calculate P(Ri > 1)
//     int rCount = 0;
//     for(int k=0; k<dataSize; ++k) {
//       if(postArray[k] > 1) rCount++;
//     }

//     outputFile << i << "\t" 
//       //<< gsl_stats_median_from_sorted_data (postArray, 1, dataSize)
// 	       << "\t" << (float)rCount / (float)dataSize << endl;

    // Output the whole posterior to a file.

    for(int k=0; k<dataSize; ++k) {
      outputFile << postArray[k] << " ";
    }
    outputFile << "\n";
  }

  outputFile.close();
  
  MPI_Finalize();
} /* end of functions */








////////// Class Implementations /////////////


Posterior::Posterior()
{
  // Constructor initialises variables
}



Posterior::~Posterior()
{

}



int Posterior::initialize(const char* filename) {

  char linebuffer[500];
  char* pch;
  mcmcFile.open(filename);
  if(!mcmcFile.is_open()) {
    cerr << "Cannot open posterior file!" << endl;
    std::abort();
  }
  vector<double> parmLine;

  while(1) {

    if(mcmcFile.eof()) break; // If we've reached the end of the file, stop
    parmLine.clear();

    mcmcFile.getline(linebuffer, 500);
 
    pch = strtok(linebuffer, " ");
    while(pch != NULL) {
      parmLine.push_back(atof(pch));
      pch = strtok(NULL," ");
    }
    posteriors.push_back(parmLine);
  }

  mcmcFile.close();

  if(posteriors.size() > 0) return 0;
  else return 1;

}



int Posterior::fetch(vector<double>& parmRow, const int& i) {
  // Fetches a row of parameters and returns them in a vector

  try
  {
    parmRow =  posteriors.at(i);
    return 0;
  }
  catch (exception& e) 
  {
    cout << "Exception: " << e.what() << endl;
    return -1;
  }

}


 
size_t Posterior::numIterations() {
   return posteriors.size();
}



EpiMath::EpiMath(EpiCovars& myEpidata)
  : epidata(myEpidata)
{
  // Registers an EpiCovars object for use in the class
  //epidata = myDataClass;

}



EpiMath::~EpiMath()
{
}



inline double EpiMath::beta_ij(const int& i, const int& j, const vector<double>& parms)
{

  double beta;

  beta = parms.at(1) * epidata.fm_Mat.isConn(i,j) * (0.5 * epidata.cFreq[j].fm * ( 3 / epidata.cFreq[j].fm_N ) );

  beta += parms.at(2) * epidata.sh_Mat.isConn(i,j) * (0.5 * epidata.cFreq[j].sh * (3 / epidata.cFreq[j].sh_N) );

  beta += parms.at(3) * epidata.cp_Mat.isConn(i,j);

  beta += parms.at(4) * exp(-parms.at(6) * (epidata.dist(i,j) - 5) ); 

  for(size_t k=7;k<16;++k) {
    if(epidata.species.at(j,k-7) == 1) {
      beta *= parms.at(k);
      break;
    }
  }

  return beta * parms.at(16);

}
