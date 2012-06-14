/* ./src/mcmc/aifuncs.h
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

/* Header file for aifuncs.cpp */

#ifndef INCLUDE_AIFUNCS_H
#define INCLUDE_AIFUNCS_H

#include <math.h>
#include <gsl/gsl_math.h>
#include <stdexcept>
#include <set>

#include "sinrEpi.h"
#include "contactMatrix.h"
#include "random.h"

using namespace std;

extern int total_pop_size;
extern double ObsTime;

/* Next we declare our parameters extern (they are declared in the main function) */

void initConnections(epiParms&, sinrEpi&);
double beta(epiParms&, sinrEpi&, int, int);
double betastar(epiParms&, sinrEpi&, int, int);
double spatialRate(epiParms&, sinrEpi&, int, int);
double networkRate(epiParms&, sinrEpi&, int, int);
double fmRate(epiParms&, sinrEpi&, int, int);
double species(epiParms&, sinrEpi&, int, int);
double binomComponent(epiParms&, infection*, double);



void drawBetaCan(const int&, const double[], gsl_matrix* const, double[], const int);
double occultProposal_pdf(const double time, const double& a, const double& b);
double occultProposal(const double& a, const double& b);

double hFunc(epiParms&, double);
double infecInteg(epiParms&, double );
double log_prod_incubLik(sinrEpi&,epiPriors&);

double compute_log_prod_pressure(epiParms&,sinrEpi&,vector<double>*);
double computeLogCT(epiParms&, sinrEpi&);
double compute_bgPress(epiParms&,sinrEpi&);
double compute_A1(epiParms&,sinrEpi&);
double compute_A2(epiParms&,sinrEpi&);



double update_log_prod(int&, epiParms&, sinrEpi&, double &, vector<double>*, vector<double>*);
double updateLogCT(int&, epiParms&, sinrEpi&, double &);
double update_bgPress(int&, epiParms&,sinrEpi&,double&);
double update_logCT(int&, epiParms&, sinrEpi&, double&);
double update_A1(int&, epiParms&, sinrEpi&, double);
double update_A2(int&, epiParms&, sinrEpi&, double&);

double addInfec_log_prod(epiParms&, sinrEpi&, double &, vector<double>*, vector<double>*);
double addInfec_bgPress(epiParms&,sinrEpi&,double);
double addInfec_logCT(epiParms&, sinrEpi&, double);
double addInfec_A1(epiParms&, sinrEpi&, double);
double addInfec_A2(epiParms&, sinrEpi&, double);

double delInfec_log_prod(int&, epiParms&, sinrEpi&, double &, vector<double>*, vector<double>*);
double delInfec_bgPress(int&,epiParms&,sinrEpi&,double);
double delInfec_logCT(int&, epiParms&, sinrEpi&, double);
double delInfec_A1(int&, epiParms&, sinrEpi&, double);
double delInfec_A2(int&, epiParms&, sinrEpi&, double);



void checkProdVec(vector<double>*,epiParms&);
double getContactStart(sinrEpi&, size_t);
void dumpContacts(sinrEpi&, size_t);
void dumpBinoms(epiParms&,sinrEpi&);
size_t numInfecByCT(sinrEpi&);
size_t numInfecContacts(sinrEpi&);

#endif
