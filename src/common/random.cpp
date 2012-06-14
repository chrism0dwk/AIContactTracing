/* ./src/common/random.cpp
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

#include "random.h"



double rng_extreme(const double &a,const double &b) 
{
  // Return simulation from f(x) = 1 - e^{-a(e^{bx} - 1)}
  return 1.0/b * log(1-log(1-gsl_rng_uniform(rng))/a);
}



double rng_extreme_pdf(const double x, const double& a, const double& b)
{
  // Returns f(x) =
  return a*b*exp(a + b*x - a*exp(b*x));
}



double rng_extreme_cdf(const double x, const double& a, const double& b)
{
  // Returns F(x) =
  return 1 - exp(-a*(exp(b*x) - 1));
}



double rng_extreme_inv_cdf(const double x, const double& a, const double &b)
{
  return 1 - rng_extreme_cdf(x,a,b);
}



void rmnorm(const int k, const double mu[], gsl_matrix* const chol, double mvNormRV[])
{
  // This function simulates from a multivariate
  // Normal distribution of dimension k.
  // The mean is supplied as array mu
  // The variance is specified as the cholesky decomposition
  //   of the variance/covariance matrix.

  double row_sum = 0.0;
  double *rv = new double[k];

  // Generate realisations of N(0,1):
  for(int i=0;i<k;++i) rv[i] = gsl_ran_gaussian(rng,1);

  // Multiply by the chol matrix and add the mean:
  for(int row=0; row<k; ++row) {
    row_sum = 0.0;
    for(int col=0; col<k; ++col) { // /*<=row*/
      row_sum += gsl_matrix_get(chol,col,row) * rv[col];
    }
    mvNormRV[row] = row_sum + mu[row];
  }

}



double truncNorm(const double mean, const double var)
{
  // Implements a rejection sampler for a left
  // truncated normal distribution

  double rv;
  rv = gsl_ran_gaussian_tail(rng,-mean,sqrt(var));

  return rv + mean;
}



double truncNorm_pdf(const double x, const double mean, const double var)
{
  // Computes the density at x of a left-truncated normal
  // distribution with mean mean and variance var

  return gsl_ran_gaussian_tail_pdf(x-mean,-mean,sqrt(var));
}
