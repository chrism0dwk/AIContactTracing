/* ./src/data/sinrEpi.h
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

#ifndef INCLUDE_SINREPI_H
#define INCLUDE_SINREPI_H

#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <gsl/gsl_math.h>
#include <stdexcept>

#include "aiTypes.hpp"
#include "contactMatrix.h"
#include "speciesMat.h"
#include "infection.hpp"
#include "SAXContactParse.hpp"

using namespace std;


class sinrEpi {

  /* Reads in data from a space separated file with cols: label | t(I) | t(N) | t(R) */

 private:

  // Private data
  int rv;

  class frequencies {
  public:
    freq_t fm;
    float fm_N;
    freq_t sh;
    float sh_N;

    frequencies();
    frequencies(const freq_t,const int,const freq_t,const int);
  };


  // Private methods
  int rhoInit(const char*,float*);
  int freqInit(const char *);
  void initContactTracing(const char* const);
  void updateInfecMethod();


 public:


  /* data */

  vector< infection* > infected;
  vector< infection* > susceptible;
  vector< infection > individuals;
  size_t N_total;
  size_t knownInfections;
  double obsTime;
  Ipos_t I1;
  float *rho;
  contactMat cp_Mat, fm_Mat, sh_Mat;
  vector<frequencies> cFreq;
  SpeciesMatrix species;

  /* Public methods */

  sinrEpi();
  ~sinrEpi();
  int init(const size_t myN_total,
		   const char* epiFile,
		   const char* contactPrefix,
		   const char* distFile,
		   const size_t nSpecies,
		   const double _obsTime);
  int addInfec(Ilabel_t,eventTime_t,eventTime_t,eventTime_t);
  int delInfec(Ipos_t);
  double exposureI(Ipos_t,Ipos_t); // Time for which j is exposed to infected i
  double exposureIBeforeCT(Ipos_t,Ipos_t);
  double ITimeBeforeCT(Ipos_t);
  double exposureN(Ipos_t,Ipos_t); // Time for which j is exposed to notified i
  double ITime(Ipos_t); // Time for which i was infective
  double NTime(Ipos_t); // Time for which i was notified
  double STime(Ipos_t); // Time for which i was susceptible
  Ipos_t updateI1(); // Updates and returns I1
  Ipos_t I2(); // Finds I2
  double sumI(); // Sum of all the infection times
  double mean_I(); // Gives the mean N - I period
  size_t numAdditions(); // Gives the current number of occult infections
  void resetOccults(); // Deletes all occults

  // Debug functions
  void dumpEpidemic();
};



class epiParms {

private:


public:

  double *beta;
  double Ican;
  double f,g;
  const int p;

  vector<float> prodCurr;
  vector<float> prodCan;
  vector<float> *prodCan_ptr;
  vector<float> *prodCurr_ptr;
  vector<float> *prodTmp_ptr;

  epiParms(const int);
  ~epiParms();
  int operator=(epiParms&);
  bool isBetaNegative();
};


class epiPriors {

 public:
  double a;
  double b;
  double* lambda;
  double* nu;

  epiPriors(int);
  ~epiPriors();
};

#endif
