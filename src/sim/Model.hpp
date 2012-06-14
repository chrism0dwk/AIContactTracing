/* ./src/sim/Model.hpp
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
 * Model.hpp
 *
 *  Created on: 14 Dec 2009
 *      Author: Chris Jewell
 */

#ifndef MODEL_HPP_
#define MODEL_HPP_

#include <vector>

#include "Parameter.hpp"




template<class Popn >
class EpiModel
{

public:

  EpiModel(Parameters* parms_, Popn* population_) : parms(parms_), population(population_) {};
  virtual ~EpiModel() {};

  // Setters - only used to change the parms and population pointers
  void setParameters(const Parameters* parms);

  // Maths
  virtual double beta(const int i, const int j) = 0;
  virtual double betastar(const int i, const int j) = 0;
  virtual double I2Npdf(const double d) = 0;
  virtual double I2Ncdf(const double d) = 0;
  virtual double I2Nrandist(const double u) = 0;

  // Data
  Parameters* parms;
  Popn* population;
};



#endif /* MODEL_HPP_ */
