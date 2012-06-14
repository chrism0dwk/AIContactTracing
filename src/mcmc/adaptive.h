/* ./src/mcmc/adaptive.h
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

/////////////////////////////////////////////////////////////////////
// Name: adaptive.cpp                                              //
// Author: C.P.Jewell                                              //
// Purpose: McmcOutput contains the output from an MCMC algorithm. //
//          It calculates the variance-covariance matrix of the    //
//           posterior at time intervals set at initialisation.    //
/////////////////////////////////////////////////////////////////////


/* CODE EXAMPLE - 5 parameters, update every 100 iterations
                  MCMC length is 100000 iterations
  #include <gsl/gsl_matrix.h>
  #include "adaptive.h"

  double mySigma[] = {1,1,1,1,1}
  McmcOutput myOutput(5,100,10000,mySigma}

  //MCMC Loop:
  for(int mcmcIter = 0; mcmcIter < 100000; ++mcmcIter) {

    // Generate proposal:

    gsl_matrix* propSD = McmcOutput.scaleChol(2.38)
    // Simulate from our proposal density using propSD

    // Accept or reject

    // Add the current state to the posterior

    McmcOutput.add(parms)
  }

*/

#ifndef _INCLUDE_ADAPTIVE_H
#define _INCLUDE_ADAPTIVE_H

#include <vector>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_statistics_double.h>
#include <gsl/gsl_errno.h>

#include "sinrEpi.h"

class McmcOutput {
private:
  double n;
  vector< vector<double> > sumOfSquares;
  vector<double> sum;

  gsl_matrix* varianceMatrix;
  gsl_matrix* cholMatrix;

  int rowCount;
  const size_t numParms, bandSize;
  bool varChanged;

  inline int doChol();
  void varianceUpdate();
  void reset();
  gsl_matrix* varCovar();

public:
  McmcOutput(const int&,const int, const int&, double[]);
  ~McmcOutput();
  void add(const epiParms&);
  void print();

  gsl_matrix* scaleChol(const double);
  bool varCheckCurr(); 
                       
};



#endif
