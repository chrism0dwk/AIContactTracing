/* ./src/mcmc/adaptive.cpp
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



#include "adaptive.h"

McmcOutput::McmcOutput(const int& p, const int n, const int& max_iter, double sigma[]) : 
    rowCount(0), 
    numParms(p), 
    bandSize(n),
    varChanged(0)
{

  // Sets up a n x p matrix to contain our band of output
  // we also set up two p x p matrices to hold the variance/covariance 
  // matrix as well as its lower triangular Cholesky decomp.

  /* Parameters:
        p - number of parameters in model
        n - the interval in iterations between variance matrix updates
        max_iter - the number of MCMC iterations requested
        sigma - an array of length p containing starting values for the proposal variances
  */

  // Vector to hold the sum
  for(int i=0; i<numParms; ++i) {
    sum.push_back(0.0);
    sumOfSquares.push_back(vector<double>(i+1,0.0));
  }

  // Variance Matrix - NB calloc sets all elements to 0
  varianceMatrix = gsl_matrix_calloc(numParms,numParms);

  // The lower triangle of this matrix will be the Cholesky decomp
  cholMatrix = gsl_matrix_calloc(numParms,numParms);

  // Fill the diagonal with our sigma^{(0)}
  for(int i=0; i<numParms; ++i) {
    gsl_matrix_set(varianceMatrix,i,i,sigma[i]);
  }

  doChol(); // And set up the cholesky matrix

}



McmcOutput::~McmcOutput() 
{
  // Clean up our dynamic memory nicely 

  gsl_matrix_free(varianceMatrix);
  gsl_matrix_free(cholMatrix);

}



inline int McmcOutput::doChol() 
{

  // Performs a Cholesky decomposition on the variance matrix
  // gsl_linalg_cholesky_decomp overwrites the input matrix with
  // the answer, so to preserve the variance matrix we have to copy it.
  // Note that if the Cholesky decomp fails, then we are left with garbage in 
  // the cholMatrix class member, so we try the decomposition in a temporary matrix
  // and copy this to cholMatrix if successful. 

  // Turn the default error handler off since we want to handle
  // any errors output by the Choleskey decomp ourselves
  gsl_set_error_handler_off();

  // Copy the variance matrix
  gsl_matrix* tempMatrix = gsl_matrix_alloc(numParms,numParms);
  gsl_matrix_memcpy(tempMatrix,varianceMatrix);

  // Attempt cholesky decomp
  if (gsl_linalg_cholesky_decomp(tempMatrix) != GSL_EDOM) { // If successful, copy result to cholMatrix
    gsl_matrix_memcpy(cholMatrix,tempMatrix);
    cout << "Cholesky done!\n";
  }
  else cout << "Cholesky not possible!\n";  // If not, then don't copy and move on.
  return(0);
}



void McmcOutput::reset()
{
  // Called when we have filled out our output band

  varianceUpdate(); // Update our empirical variance/covariance matrix
  rowCount = 0; // Reset the row counter to 0 for the new output band
}



void McmcOutput::varianceUpdate() 
{
  // Recalculates the variance-covariance matrix

  double var = 0.0; // Our variance or covariance 

  if((rowCount % bandSize) != 0 || rowCount == 0) return; // If we're not ready to update, don't!

  // We now iterate over the rows and columns of the variance matrix

  for(int i=0; i<numParms; ++i) {

    var = sumOfSquares[i][i]/rowCount - sum[i]/rowCount * sum[i]/rowCount;
    gsl_matrix_set(varianceMatrix,i,i,var);

    for(int j=0; j<i; ++j) {
      var = sumOfSquares[i][j]/rowCount - sum[i]/rowCount * sum[j]/rowCount;
      gsl_matrix_set(varianceMatrix,i,j,var);
      gsl_matrix_set(varianceMatrix,j,i,var);
    }

  }

  varChanged = 1;
}



void McmcOutput::add(const epiParms& parms)
  // Adds a new row of MCMC output to the class data member
  // and calls varianceUpdate() at intervals set by bandSize
{

  // Add to sum vector and sum of squares matrix
  for(int i=0; i<numParms; ++i) {
    sum[i] += parms.beta[i];
    sumOfSquares[i][i] += parms.beta[i] * parms.beta[i];
    for(int j=0; j < i; ++j) {
      sumOfSquares[i][j] += parms.beta[i]*parms.beta[j];
    }
  }

  rowCount++;

  if((rowCount % bandSize) == 0 && rowCount != 0) {
    varianceUpdate();
  }
}



gsl_matrix* McmcOutput::varCovar()
{
  // Returns a pointer to the variance/covariance matrix

  return varianceMatrix;
}



bool McmcOutput::varCheckCurr() 
{
  // Allows the user to check whether the variance-covariance
  // matrix has changed since he/she last reset the flag.

  return varChanged;
}



gsl_matrix* McmcOutput::scaleChol(const double scaleFactor) 
{
  // Returns a pointer to a matrix containing the Cholesky decomposition
  // of the scaled variance matrix

  if(varChanged == 1) {
    gsl_matrix_scale(varianceMatrix,scaleFactor);
    doChol();
    varChanged = 0;
  }

  return cholMatrix;
}



void McmcOutput::print()
{
  // Prints out the variance matrix
  // ONLY USED FOR DEBUGGING PURPOSES

  for(int i=0;i<numParms;++i) {
    for(int j=0;j<numParms;++j) {
      printf("%f ", gsl_matrix_get(varianceMatrix,j,i));
    }
    cout << "\n";
  }
}
