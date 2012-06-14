/* ./src/sim/SellkeSim.cpp
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
 * SellkeSim.cpp
 *
 *  Created on: 4 Dec 2009
 *      Author: stsiab
 */

#include "SellkeSim.hpp"

SellkeSim::SellkeSim(const int nTotal_,
                     const double tObs_,
                     const string epiFilename_,
                     const string dataPrefix_)
{
  // Simulates epidemics using the Sellke construction

  // First population the dataset
  data = new AIPopulation(nTotal_,tObs_);

}

SellkeSim::~SellkeSim() {
	// TODO Auto-generated destructor stub
}
