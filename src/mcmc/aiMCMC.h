/* ./src/mcmc/aiMCMC.h
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

/* MCMC code for Bird Flu epidemic */

/* USAGE:

    aiMCMC <epi data> <location file> <total popn size> <total no infected> <no of iterations> <output file>
*/

#ifndef SRC_AIMCMC_H
#define SRC_AIMCMC_H

#include <math.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdlib.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_statistics_double.h>
#include <time.h>
#include <list>
#include <cassert>
#include <omp.h>

#include "aifuncs.h"
#include "sinrEpi.h"
#include "contactMatrix.h"
#include "adaptive.h"
#include "random.h"
#include "occultWriter.h"

/* Constants */

#define DIM_PARMS 16


using namespace std;

int fileInit(char*,float*,int);
int main(int, char**);
int inputConf();

// Global variables

int rv; // Generic return value
gsl_rng *rng;
char config_filename[200];
char epidataFile[200];
char loc_filename[200];
char output_filename[200];
char cMat_prefix[200];
bool block_update = false;
int seed = 1;
int max_iter;
int burnIn,thin;
epiParms parms(DIM_PARMS);
epiParms parms_can(DIM_PARMS);
epiParms parmsTemp(DIM_PARMS);
epiPriors priors(DIM_PARMS);
sinrEpi epidata;
double sigma_mult[DIM_PARMS];
double sigma_add[DIM_PARMS];
double a_m_ratio;
int addOffset;
double ObsTime;
time_t t_start, t_end;
int infecFiddle;
double xi;


#endif
