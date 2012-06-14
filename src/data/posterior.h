/* ./src/data/posterior.h
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

#ifndef POSTERIOR_H
#define POSTERIOR_H


#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

/**
	@author Chris Jewell <chris.jewell@warwick.ac.uk>
*/
class Posterior
  {
    //! This class sets up a 2D array to hold the posteriors
    //! See below for implementation

  private:
    vector< vector<double> > posteriors;
    ifstream mcmcFile;

  public:
    Posterior();
    ~Posterior();
    int initialize(const char*, const int skip=1);
    int fetch(vector<double>&, const int&);
    size_t numIterations();
  };

#endif
