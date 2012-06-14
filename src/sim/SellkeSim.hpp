/* ./src/sim/SellkeSim.hpp
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
 * SellkeSim.hpp
 *
 *  Created on: 4 Dec 2009
 *      Author: Chris Jewell
 */

#ifndef SELLKESIM_HPP_
#define SELLKESIM_HPP_

#include <string>

#include "AIPopulation.hpp"
#include "aiModel.hpp"

class SellkeSim;


// Alias infection as Individual, makes more sense!


class SellkeSim {
public:
	SellkeSim(const int nTotal_,
                  const double tObs_,
                  const string epiFilename_,
		  const string dataPrefix_);
	virtual ~SellkeSim();

private:
	AIPopulation* data;
	AIModel* model;

};







#endif /* SELLKESIM_HPP_ */
