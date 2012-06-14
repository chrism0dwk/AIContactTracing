/* ./src/common/random.h
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

#ifndef _INCLUDE_RANDOM_H
#define _INCLUDE_RANDOM_H

#include <gsl/gsl_math.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

extern gsl_rng *rng;

double rng_extreme(const double&, const double&);
double rng_extreme_pdf(const double, const double&, const double&);
double rng_extreme_cdf(const double, const double&, const double&);
double rng_extreme_inv_cdf(const double, const double&, const double&);

void rmnorm(const int, const double[], gsl_matrix* const, double[]);
double truncNorm(const double, const double);
double truncNorm_pdf(const double, const double, const double);


#endif
