/* ./src/sim/aiModel.cpp
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

/*
 * aiModel.cpp
 *
 *  Created on: 11 Dec 2009
 *      Author: Chris Jewell
 */

#include <gsl/gsl_math.h>
#include <string.h>

#include "aiModel.hpp"



AIModel::AIModel(Parameters* parms_, AIPopulation* population_) : EpiModel<AIPopulation>(parms_, population_)
{
  // Initialise members
}



AIModel::~AIModel()
{
  // Dtor
}



double AIModel::speciesSusc(const size_t j)
{
  double susc = 1.0;
  AIPopulation::indivtype* indiv = (*population)[j];

  for(int k = 0; k < NSPECIES; ++k) {
    if (indiv->species[k] == true) {
      susc *= parms->at(k+7).value;
      break;
    }
  }

  return susc;
}



double AIModel::fmRate(const int i, const int j)
{
  // Feedmill rate
  double rate = 10 * population->fmContact.isConn(i,j) * ( 0.5 * (*population)[j]->fm * ( 3.0 / ((*population)[j]->fm_N) ) );
  return rate;
}



double AIModel::shRate(const int i, const int j)
{
  // Slaughterhouse rate
  double rate = 10 * population->shContact.isConn(i,j) * ( 0.5 * (*population)[j]->sh * ( 3.0 / ((*population)[j]->sh_N) ) );
  return rate;
}



double AIModel::cpRate(const int i, const int j)
{
  // Company rate
  double rate = parms->at(3).value*population->cpContact.isConn(i,j);
  return rate;
}



double AIModel::iSpatRate(const int i, const int j)
{
  // Spatial rate I->S
  double rate = parms->at(4).value*exp(-parms->at(6).value*(population->getRho(i,j) - 5.0));
  return rate;
}



double AIModel::nSpatRate(const int i, const int j)
{
  // Spatial rate N->S
  double rate = parms->at(5).value*exp(-parms->at(6).value*(population->getRho(i,j) - 5.0));
  return rate;
}


double AIModel::beta(const int i, const int j)
{
  double betaij = 0.0;

  betaij = fmRate(i,j) + shRate(i,j) + cpRate(i,j) + iSpatRate(i,j);
  betaij *= this->speciesSusc(j);

  return betaij;
}



double AIModel::betastar(const int i, const int j)
{
  double betaij = 0.0;

  betaij += nSpatRate(i,j);
  betaij *= this->speciesSusc(j);

   return betaij;
}



double AIModel::I2Npdf(const double d)
{
  double a = parms->at(17).value;
  double b = parms->at(18).value;

  return a*b*exp(a + b*d - a*exp(b*d));
}



double AIModel::I2Ncdf(const double d)
{
  double a = parms->at(17).value;
  double b = parms->at(18).value;

  return 1 - exp(-a*(exp(b*d) - 1));
}



double AIModel::I2Nrandist(const double u)
{
  double a = parms->at(17).value;
  double b = parms->at(18).value;

  return 1.0/b * log(1 - log( 1 - u )/a);
}

