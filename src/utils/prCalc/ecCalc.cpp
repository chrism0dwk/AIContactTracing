/* ./src/utils/prCalc/ecCalc.cpp
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
// Name: ecCalcPR                                                         //
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

// GSL includes
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_blas.h>

// Local includes
#include "epiCovars.h"
#include "speciesMat.h"

// MPI
#include <mpi.h>

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
  MPI_Comm_size(MPI_COMM_WORLD, &mpiWorldSize);
  MPI_Comm_rank(MPI_COMM_WORLD, &myId);

  cerr << "Starting with " << N_total << " individuals" << endl;

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



  // Allocate the tx-rate matrix
  gsl_matrix *txMatrix = gsl_matrix_alloc(N_total,N_total);



  // Declare PageRank variables:
  double colSum; // Required to normalise the columns
  gsl_vector *R;
  gsl_vector *R1;
  gsl_vector *E;
  gsl_vector *workSpace;


  // Open the output file
  sprintf(outputFileName,"%s.%i.csv",outputPrefix,myId);
  outputFile.open(outputFileName); // Open the output file


  //////////////////////////////////////////////////////////////////////
  // Begin loop over the posteriors
  /////////////////////////////////////////////////////////////////////

  for(size_t parmSet=startParmSet; parmSet < endParmSet; parmSet += thinBy) {

    cerr << "Initialising param set " << parmSet;

    posteriors.fetch(parms,parmSet); // Just fetch one line for now.
    sparseCounter = 0.0;

    for (int col=0; col<N_total; ++col) {
      
      for (int row=0; row<N_total; ++row) {  // Loop over the rows
	betaij = mathHandler.beta_ij(row, col, parms);
	gsl_matrix_set(txMatrix, row, col, betaij);
	if(betaij == 0.0) sparseCounter++;
      }
      
      gsl_matrix_set(txMatrix, col, col, 0.0);// Diagonal is 0 since you can't
      // contact yourself
     
    }
    
    cerr << "Done (Sparseness: " << (double)sparseCounter / ((double)N_total*(double)N_total) * 100.0 << "%)" << "\n";
    
    
    ///////////////////////////// PageRank algorithm \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
      
      cerr << "Calculating PageRank solution....\n";
      
      
      // Allocate storage vectors
      
      R = gsl_vector_alloc(N_total);
      R1 = gsl_vector_alloc(N_total);
      E = gsl_vector_alloc(N_total);
      workSpace = gsl_vector_alloc(N_total);
      
      double delta,epsilon;
      double vecSum;
      
      // Initialise R and E
      
      gsl_vector_set_all(R,1.0/(double)N_total);
      gsl_vector_set_all(R1,1.0/(double)N_total);
      gsl_vector_set_all(E,1.0/(double)N_total);
      
      // Epsilon is our required accuracy
      
      epsilon = 0.0001;
      delta = epsilon + 1; // Start value for delta
      
      
      // Begin the PageRank loop
      
      while(delta > epsilon) {
	
	// R_{i+1} <- AR_i
	gsl_blas_dgemv(CblasNoTrans, 1, txMatrix, R, 0, R1);
	
	// Normalise the R1 vector.
	vecSum = gsl_blas_dnrm2(R1);
	gsl_vector_scale(R1,1/vecSum);
	
	// \delta <- ||R_{i+1} - R_i||_1
	gsl_blas_dcopy(R1,workSpace);
	gsl_vector_sub(workSpace,R);
	delta = gsl_blas_dasum(workSpace);
	
	// R_i <- R_{i+1}
	gsl_blas_dcopy(R1,R);
	
	if( delta == DBL_MAX ) {
	  throw overflow_error("Overflow in delta.  PageRank non-convergence!\n");
	}
	
	cerr << "Delta=" << delta << "\n";
	
      } //// End PageRank Loop \\\\
      

      // Print a result row to the output file
      for(int row=0; row<N_total; ++row) {
	outputFile << gsl_vector_get(R,row) << " ";
      }
      outputFile << "\n";
      
      
  } /////// Loop over posteriors \\\\\\\\


  // Cleanup

  outputFile.close();

  gsl_matrix_free(txMatrix);
  gsl_vector_free(R);
  gsl_vector_free(R1);
  gsl_vector_free(E);
  gsl_vector_free(workSpace);


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
