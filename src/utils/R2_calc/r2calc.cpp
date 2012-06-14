/* ./src/utils/R2_calc/r2calc.cpp
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

////////////////////////////////////////////////////////////////////////////
//                                                                        //
// Name: r2calc                                                           //
// Author: C Jewell                                                       //
// Created: 28.06.2007                                                    //
// Purpose: To calculate PageRank scores for premises in an epidemic      //
//                                                                        //
// References:                                                            //
//                                                                        //
//@techreport{ page98pagerank,                                            //
//    author = "Lawrence Page and Sergey Brin and Rajeev Motwani \        //
//              and Terry Winograd",                                      //
//    institution = "Stanford Digital Library Technologies Project",      //
//    title = "The PageRank Citation Ranking: Bringing Order to the Web", //
//    year = "1998",                                                      //
//    url = "citeseer.ist.psu.edu/page98pagerank.html" }                  //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

// CStdLib
#include <iostream>
#include <ostream>
#include <vector>
#include <exception>
#include <math.h>
#include <cfloat>
#include <climits>
#include <string.h>

// Local includes
#include "epiCovars.h"
#include "speciesMat.h"

// MPI
#ifdef __MPI__
  #include <mpi.h>
#endif

using namespace std;

///////////// Lapack definitions \\\\\\\\\\\\\\\\

extern "C" {
  extern void dgesv_(int*, int*, double*, int*,
		    int*, double*, int*, int*);

  extern void dgemv_(char*, int*, int*, double*,
		    double*, int*, double*, int*,
		    double*, double*, int*);

}


///////////// Class Definitions \\\\\\\\\\\\\\\\\

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

#ifdef __MPI__
  MPI_Init(&argc, &argv);
#endif

  if (argc != 8) {
    cout << "USAGE: ecCalc <MCMC output> <burnin> <skip> <distances> <contact prefix> <population size> <out file prefix> <task id> " << endl;
    return -1;
  }
  
  
  // Declarations //
  
  const char* const mcmcOutput = argv[1];
  const int N_total = atoi(argv[6]);
  const int burnIn = atoi(argv[2]);
  const int thinBy = atoi(argv[3]);
  const char* const distanceFile = argv[4];
  const char* const contactPrefix = argv[5];
  const char* const outputPrefix = argv[7];
  vector<double> parms;
  int sparseCounter;
  ofstream outputFile;
  char outputFileName[50];
  int bandSize,startParmSet,endParmSet;

  double betaij; // Holds the current beta_ij

  // MPI things
  int myId, mpiWorldSize;

#ifdef __MPI__
  MPI_Comm_size(MPI_COMM_WORLD, &mpiWorldSize);
  MPI_Comm_rank(MPI_COMM_WORLD, &myId);
#else
  myId = 0;
  mpiWorldSize = 1;
#endif

  if(myId == 0) {
    cerr << "Starting with " << N_total << " individuals" << endl;
  }

  EpiCovars covars;  // Class to hold our covariates
  Posterior posteriors;
  EpiMath mathHandler(covars); // Sets up the maths handler


  // Initialise the covariates //
  
  covars.init(N_total,contactPrefix,distanceFile);


  // Initialise the posterior object //

  if(posteriors.initialize(mcmcOutput) != 0) {
    cerr << "Error initialising posteriors!\n";
    std::abort();
  }


  // Calculate the bandsize to distribute the farms
  // among the MPI nodes

  if(myId == 0) {
    cerr << "Got " << posteriors.numIterations() << " realisations of the joint posterior" << endl;
  }

  bandSize = (posteriors.numIterations() - burnIn) / mpiWorldSize;

  startParmSet = myId * bandSize + burnIn;
  if(myId != mpiWorldSize - 1)
    endParmSet = startParmSet + bandSize - 1;
  else
    endParmSet = posteriors.numIterations() - 1;


  // Allocate matrix storage structures

  double* txMatrix;
  double* A1;
  double* R;
  int info;
  int* ipiv;


  try {
    // Allocate the tx-rate matrix
    txMatrix = new double[N_total*N_total];
    ipiv = new int[N_total];
    A1 = new double[N_total];
    R = new double[N_total];
  }
  catch (bad_alloc&) {
    cerr << "Cannot allocate storage structures.  Check available memory" << endl;
    abort();
  }

  // Open the output file
  sprintf(outputFileName,"%s.%02i.csv",outputPrefix,myId);
  outputFile.open(outputFileName); // Open the output file


  //////////////////////////////////////////////////////////////////////
  // Begin loop over the posteriors
  /////////////////////////////////////////////////////////////////////

  for(size_t parmSet=startParmSet; parmSet < endParmSet; parmSet += thinBy) {

    cerr << "Initialising param set " << parmSet << "\n";

    posteriors.fetch(parms,parmSet); // Just fetch one line for now.
    sparseCounter = 0.0;


    cerr << "Calculating solution....\n";


    // Create txMatrix:
    for (int col=0; col<N_total; ++col) {
      for (int row=0; row<N_total; ++row) {  // Loop over the rows
	betaij = mathHandler.beta_ij(row, col, parms);
	*(txMatrix + row + N_total*col) = betaij; //Col-major for FORTRAN
	                                          //   routines.
	if(betaij == 0.0) sparseCounter++;
      }
      
      *(txMatrix + col +  col*N_total) = 0.0; // Diagonal is 0 since you can't
                                              //   contact yourself
         
    }
    
    cerr << "txMatrix created (Sparseness: "
	 << (double)sparseCounter / ((double)N_total*(double)N_total) * 100.0
	 << "%)" << "\n";
    
    
    // Begin the LaPack operations
    
    // a. Calculate A1 which contains all the \beta_i's

    double rowSum = 0.0;

    for(size_t i=0; i<N_total; ++i) {
      rowSum = 0.0;
      for(size_t j=0; j<N_total; ++j) {
	rowSum += *(txMatrix + i + j*N_total);
      }
      A1[i] = rowSum;  // \sum_j \beta_{ij}
    }


    // b. For each premises, create a weighted sum of R_I^(2)
    
    for(size_t i=0; i<N_total; ++i) {
      R[i] = 0.0;
      for(size_t j=0; j<N_total; ++j) {
	if(i==j)
	  R[i] += A1[i] * parms.at(16);  // R_i
	else 
	  R[i] += A1[j] * parms.at(16) * (*(txMatrix + i + j*N_total) / A1[i]);
      }

    }




    // Print a result row to the output file
    for(int row=0; row<N_total; ++row) {
      outputFile << R[row] << " ";
    }
    outputFile << endl;
    
    
  } /////// Loop over posteriors \\\\\\\\


  // Cleanup

  outputFile.close();

  delete txMatrix;
  delete ipiv;
  delete A1;
  delete R;


#ifdef __MPI__
  MPI_Finalize();
#endif


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
    cerr << "Exception: " << e.what() << endl;
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

  return beta;

}
