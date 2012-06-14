/* ./src/sim/main.cpp
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
 * main.cpp
 *
 *  Created on: 14 Dec 2009
 *      Author: Chris Jewell
 */


#include <cstdlib>
#include <string>

#include "aiModel.hpp"
#include "Parameter.hpp"

class SellkeSim
{

};


int main(int argc, char* argv[]) {


  const size_t N_total(8636);
  const double obsTime(50.0);

  Parameters parms;
  AIPopulation population(8636,50.0,"/home/stsiab/AI/trunk/MCMC_input/kieran");

  AIModel model(&parms, &population);

  population.loadEpiData("/home/stsiab/AI/trunk/MCMC_input/ss.18.0.1.ipt");

  return EXIT_SUCCESS;
}
