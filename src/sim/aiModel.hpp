/* ./src/sim/aiModel.hpp
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
 * aiModel.hpp
 *
 *  Created on: 11 Dec 2009
 *      Author: stsiab
 */

#ifndef AIMODEL_HPP_
#define AIMODEL_HPP_

#include <string>

#include "Model.hpp"
#include "AIPopulation.hpp"
#include "speciesMat.h"
#include "contactMatrix.h"





class AIModel : public EpiModel<AIPopulation>
{
  //! Contains the Avian Influenza population

public:

  AIModel(Parameters* parms_, AIPopulation* population_);
  ~AIModel();

  // Maths
  double fmRate(const int i, const int j);
  double shRate(const int i, const int j);
  double cpRate(const int i, const int j);
  double iSpatRate(const int i, const int j);
  double nSpatRate(const int i, const int j);
  double beta(const int i, const int j);
  double betastar(const int i, const int j);
  double I2Npdf(const double d);
  double I2Ncdf(const double d);
  double I2Nrandist(const double u);
  double speciesSusc(const size_t j);

};

#endif /* AIMODEL_HPP_ */
