/* ./src/sim/simContacts.cpp
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
 * simContacts.cpp
 *
 *  Created on: 4 Jan 2010
 *      Author: stsiab
 *     Purpose: To simulate contacts between individuals in a network
 *              according to a Poisson process conditional on individual-level
 *              covariates.
 */

// Standard
#include <cstdlib>
#include <iostream>

// GSL
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

// XML
#include "XmlCTWriter.hpp"

// AIPopulation
#include "aiModel.hpp"
#include "Parameter.hpp"



// CONSTANTS
const int NTOTAL(8636);
const float OBSTIME(500.0);



using namespace std;
using namespace xercesc;



int main(int argc, char* argv[])
{

  if (argc != 4) {
    cout << "Usage: simContacts <data prefix> <XML output file> <seed>"  << endl;
    return 1;
  }

  char* xmlFilename = argv[2];
  char* dataPrefix = argv[1];
  int seed = atoi(argv[3]);


  // XML Framework
  XmlCTWriter* ctWriter;
  ctWriter = new XmlCTWriter();


  // Set up population
  AIPopulation popn(NTOTAL,OBSTIME,dataPrefix);
  Parameters parms;

  parms.push_back(Parameter(1e-6)); // Beta0
  parms.push_back(Parameter(1));  // p1 - dummy, fix to 1
  parms.push_back(Parameter(1));  // p2 - dummy, fix to 1
  parms.push_back(Parameter(0.008));// beta1
  parms.push_back(Parameter(0.018));// beta2
  parms.push_back(Parameter(0.0074));// beta3
  parms.push_back(Parameter(0.2)); // psi
  parms.push_back(Parameter(0.6)); // CLayers
  parms.push_back(Parameter(0.3)); // DLayers
  parms.push_back(Parameter(0.3)); // DMeat
  parms.push_back(Parameter(0.3)); // GLayers
  parms.push_back(Parameter(0.3)); // GMeat
  parms.push_back(Parameter(0.3)); // Partridge
  parms.push_back(Parameter(0.3)); // Pheasant
  parms.push_back(Parameter(0.3)); // Quail Layers
  parms.push_back(Parameter(0.3)); // Turkey


  AIModel model(&parms, &popn);



  /* Simulation algorithm:
   *
   * Receiver j, sender i;
   *
   * Foreach j:
   *
   *   // Do background
   *   Exp(beta0)
   *
   *   Foreach i:
   *     Foreach k in networks:
   *       t = 0;
   *       while(t < tObs):
   *         t' = t + Exp(betaijk);
   *         Store t' in contact tracing for j
   *
   */

  gsl_rng* rng = gsl_rng_alloc(gsl_rng_mt19937);
  gsl_rng_set (rng, seed);
  double time;
  double beta;

  for(int j = 0; j < NTOTAL; j++) {

    XmlCTData* ctData = ctWriter->createCTData(j);
    ctData->setCTStartTime(0.0);

    // Get species susceptibility
    double suscep = 1.0;
    for(size_t k = 0; k<NSPECIES; k++) {
      if((*popn[j]).species[k] == true) {
        suscep *= parms[k+7].value;
        break;
      }
    }

    cout << "Suscep = " << suscep << endl;
    // Background
    time = 0.0;
    while(time < OBSTIME) {
      time += gsl_ran_exponential(rng, 1 / parms[0].value);
      if (time > OBSTIME) break;
      ctData->appendContact(9000,true,"background",time,false);
    }

    // Feedmills
    for(int i = 0; i < NTOTAL; i++) {
      if (i == j) continue;

      time = 0.0;
      beta = model.fmRate(i,j) * model.speciesSusc(j);
      //if(beta > 0) cout << "beta(" << i << "," << j << ") = " << beta << endl;
      while(time < OBSTIME) {
        time = time + gsl_ran_exponential(rng, 1 / beta);
        if (time > OBSTIME) break;
        ctData->appendContact(i, true, "feedmill", time, false);
      }
    }

    // Slaughterhouse
    for(int i = 0; i < NTOTAL; i++) {
      if (i == j) continue;

      time = 0.0;
      beta = model.shRate(i,j) * model.speciesSusc(j);
      //if (beta > 0) cout << "beta(" << i << "," << j << ") = " << beta << endl;
      while(time < OBSTIME) {
        time = time + gsl_ran_exponential(rng, 1 / beta);
        if (time > OBSTIME) break;
        ctData->appendContact(i, true, "shouse", time, false);
      }
    }

    // Company
    for(int i = 0; i < NTOTAL; i++) {
      if (i == j) continue;

      time = 0.0;
      beta = model.cpRate(i,j) * model.speciesSusc(j);
      while(time < OBSTIME) {
        time = time + gsl_ran_exponential(rng, 1 / beta);
        if (time > OBSTIME) break;
        ctData->appendContact(i, true, "company", time, false);
      }
    }

    // Spatial if I->S
    for(int i = 0; i < NTOTAL; i++) {
      if (i == j) continue;

      time = 0.0;
      beta = model.iSpatRate(i,j) * model.speciesSusc(j);
      while(time < OBSTIME) {
        time = time + gsl_ran_exponential(rng, 1 / beta);
        if (time > OBSTIME) break;
        ctData->appendContact(i, true, "ispatial", time, false);
      }
    }

    // Spatial if N->S
    for(int i = 0; i < NTOTAL; i++) {
      if (i == j) continue;

      time = 0.0;
      beta = model.nSpatRate(i,j) * model.speciesSusc(j);
      while(time < OBSTIME) {
        time = time + gsl_ran_exponential(rng, 1 / beta);
        if (time > OBSTIME) break;
        ctData->appendContact(i, true, "nspatial", time, false);
      }
    }

    // Add CT Data
    ctWriter->addCTData(ctData);
  }


  // Now write CT Data to file
  ctWriter->writeToFile(xmlFilename);


  // Clean up
  delete ctWriter;

  // Done
  return 0;
}

