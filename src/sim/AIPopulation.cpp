/* ./src/sim/AIPopulation.cpp
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
 * AIPopulation.cpp
 *
 *  Created on: 17 Dec 2009
 *      Author: stsiab
 */

#include <gsl/gsl_math.h>
#include <string.h>
#include <cstdlib>

#include "AIPopulation.hpp"
#include "speciesMat.h"


AIIndividual::AIIndividual(const size_t label_,
                           const double I_,
                           const double N_,
                           const double R_,
                           const bool isKnown,
                           const infecStatus_e _status) : Individual(label_,
                                                                     I_,
                                                                     N_,
                                                                     R_,
                                                                     isKnown,
                                                                     _status)
{

}



AIPopulation::AIPopulation(const size_t N_total_,
                           const double obsTime_,
                           const string dataPrefix) : Population< AIIndividual >(N_total_, obsTime_)
{
  loadData(dataPrefix);
}



AIPopulation::~AIPopulation()
{

}



void AIPopulation::loadData(const string dataPrefix)
{
   //! Loads covariate data into AIModel class

   string filename;

   // Load feedmill contact data
   filename = dataPrefix + ".fm";
   fmContact.init(filename.c_str(),N_total);

   // Load slaughterhouse contact data
   filename = dataPrefix + ".sh";
   shContact.init(filename.c_str(),N_total);

   // Load company contact data
   filename = dataPrefix + ".cp";
   cpContact.init(filename.c_str(),N_total);

   // Load contact frequency data
   filename = dataPrefix + ".freq";
   freqInit(filename);

   // Initialise distance matrix
   filename = dataPrefix + "_dist.txt";
   calcRho(filename);

   // Initialise species
   filename = dataPrefix + ".sp";
   initSpecies(filename);
}



void AIPopulation::calcRho(const string filename)
{
  //! Calculates the distance matrix

  ifstream datafile;
  int i,j;
  float dist;
  char line[200];
  char element[10];
  char *line_ptr;
  char *element_ptr;

  cout << "Reading distance matrix from: " << filename.c_str() << endl;

  // Allocate
  try {
    rho = new float[N_total*N_total];
  }
  catch (exception& e) {
    cerr << "Error allocating distance matrix: " << e.what() << endl;
    std::abort();
  }

  cout << "Initialising distance matrix..." << flush;

  // Set all elements of rho to GSL_POS_INF:
#pragma omp parallel for default(shared) private(i,j) schedule(static)
  for(int i=0; i < N_total;++i) {
    for(int j=0; j < N_total; ++j) {
      *(rho+i+N_total*j) = GSL_POSINF;
    }
  }

  cout << "Done" << endl;

  datafile.open(filename.c_str(),ios::in);
  if(!datafile.is_open()) {
    throw runtime_error("Cannot open distance file");
  }

  while(1) {

    datafile.getline(line,200);
    if(datafile.eof()) break;
    if(strlen(line) <= 1) break;
    sscanf(line,"%i %i %f",&i,&j,&dist);
    *(rho + i + N_total*j) = dist;
  }

  datafile.close();

}



double AIPopulation::getRho(const size_t i, const size_t j) const
{
  // Fetches an entry in distance matrix
  return *(rho+i+N_total*j);
}



void AIPopulation::initSpecies(const string filename)
{
  //! Initialises the species

  cout << "Reading species from '" << filename.c_str() << "'..."<< flush;
  SpeciesMatrix* rawMat = new SpeciesMatrix;

  rawMat->initialize(filename.c_str(), N_total, NSPECIES);

  // Iterate through rows and put species counts into individuals
  for(int i = 0; i<N_total; ++i) {
    for(int mySpecies = 0; mySpecies < NSPECIES; ++mySpecies) {
      individuals[i].species[mySpecies] = rawMat->at(i,mySpecies);
    }
  }

  delete rawMat;

  cout << "Done" << endl;
}



void AIPopulation::freqInit(const string filename)
{
  //! Initialises contact frequency data

  cout << "Reading contact frequencies from '" << filename.c_str() << "'...'" << flush;

  ifstream datafile;
  datafile.open(filename.c_str(),ios::in);
  if(!datafile.is_open()) {
    throw runtime_error("Cannot open frequency file");
  }

  size_t counter = 0;
  char line[200];
  while(1) {
    datafile.getline(line,200);
    if(datafile.eof()) break;
    if(strlen(line) <= 1) break;
    sscanf(line,"%lf %lu %lf %lu",&(individuals[counter].fm),&(individuals[counter].fm_N),&(individuals[counter].sh),&(individuals[counter].sh_N));
    counter++;
  }
  if(counter != N_total) {
    throw runtime_error("Garbled frequency file");
  }

  datafile.close();

  cout << "Done" << endl;
}
