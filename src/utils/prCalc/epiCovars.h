/* ./src/utils/prCalc/epiCovars.h
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

/* Header file for sinrEpi class - contains a SINR epidemic */

#ifndef INCLUDE_EPICOVARS_H
#define INCLUDE_EPICOVARS_H
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "contactMatrix.h"
#include <gsl/gsl_math.h>

#include "speciesMat.h"

using namespace std;

typedef unsigned int Slabel_t;
typedef unsigned int Spos_t;
typedef unsigned int Ilabel_t;
typedef unsigned int Ipos_t;
typedef float freq_t;
typedef float eventTime_t;



class EpiCovars {

  /* Reads in data from a space separated file with cols: label | t(I) | t(N) | t(R) */

 private:

  int rhoInit(const char*,float*);
  int freqInit(char *);
  int rv;

  class frequencies {
  public:
    freq_t fm;
    float fm_N;
    freq_t sh;
    float sh_N;
  };

 public:

  int N_total;
  float *rho;
  contactMat cp_Mat, fm_Mat, sh_Mat;
  vector<frequencies> cFreq;
  SpeciesMatrix species;


  /* Public methods */

  EpiCovars();
  ~EpiCovars();
  int init(const int, const char*, const char*);
  double dist(const int&,const int&);
};

#endif
