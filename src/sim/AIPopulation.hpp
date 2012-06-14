/* ./src/sim/AIPopulation.hpp
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
 * AIPopulation.hpp
 *
 *  Created on: 17 Dec 2009
 *      Author: stsiab
 */

#ifndef AIPOPULATION_HPP_
#define AIPOPULATION_HPP_

#include "Population.hpp"
#include "contactMatrix.h"
#include "Individual.hpp"

using namespace EpiRisk;


// Constants
const size_t NSPECIES(9);


class AIIndividual : public EpiRisk::Individual
{
  //! Provides class for Avian Influenza individual

public:
  // Virtual constructors
  AIIndividual(const size_t label,
               const double I,
               const double N,
               const double R,
               const bool isKnown=0,
               const infecStatus_e _status=SUSCEPTIBLE);

  bool species[NSPECIES];
  double fm, sh;
  size_t fm_N, sh_N;
};


class AIPopulation : public Population< AIIndividual >
{
public:
  AIPopulation(const size_t N_total_,
               const double obsTime_,
               const string dataPrefix);
  virtual
  ~AIPopulation();

  // Data
  float* rho; //Distance matrix
  contactMat fmContact;
  contactMat shContact;
  contactMat cpContact;

  double getRho(const size_t i, const size_t j) const;

private:
  void loadData(const string dataPrefix);

  void calcRho(const string filename);
  void initSpecies(const string filename);
  void freqInit(const string filename);

};


#endif /* AIPOPULATION_HPP_ */
