/* ./src/sim/gillespie/GillespieSim.hpp
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
/* Simulation class for Avian Influenza Data Simulation */

#ifndef _INCLUDE_AISIM_H
#define _INCLUDE_AISIM_H

#include<iostream>
#include<iomanip>
#include<fstream>
#include<math.h>
#include<vector>
#include<list>
#include<gsl/gsl_rng.h>
#include<gsl/gsl_randist.h>
#include<gsl/gsl_math.h>
#include<stdexcept>
#include<exception>
#include<map>
#include<cassert>

// Custom headers
#include "contactMatrix.h"
#include "speciesMat.h"

#include "XmlCTWriter.hpp"


// Fwd decls

class Individual;


using namespace std;

typedef float freq_t;



// Typedefs

// Types
enum EVENTTYPE {
  CONTACTEVENT=1,
  INFECTIONEVENT,
  NOTIFICATIONEVENT,
  REMOVALEVENT,
  NOEVENT
};

//
//
//enum STATUS {
//  SUSCEPTIBLE=1,
//  INFECTED,
//  NOTIFIED,
//  REMOVED
//};



enum CONTYPE {
  FEEDMILL,
  SHOUSE,
  OTHER
};



typedef struct {
  int id;
  double time;
} LABEL;



typedef struct {
  Individual* sender;
  Individual* receiver;
  CONTYPE method;
  double time;
} CONTACT;



typedef struct {
  EVENTTYPE type;
  double time;
} EVENT;



typedef struct {
  size_t label;
  double time;
  CONTYPE type;
} CONINFO;



class Individual {
 public:

  enum INFECSTATUS { SUSCEPTIBLE=0,
         INFECTED,
         NOTIFIED,
         REMOVED };

  size_t label;
  double I;
  double N;
  double R;
  bool isDC;
  INFECSTATUS status;
  size_t infecBy;
  CONTYPE infecType;
  XmlCTData* ctData;

  Individual(size_t,double);
  bool isSAt(const double time) const;
  bool isIAt(const double time) const;
  bool isNAt(const double time) const;
  bool isRAt(const double time) const;

  INFECSTATUS statusAt(const double time) const;
};



typedef struct {
  size_t label;
  double time;
  Individual::INFECSTATUS status;
} STATLABEL;



typedef map<double,Individual*> B_INDEX;


class INRTuple {
 public:
  double I;
  double N;
  double R;
};



class GillespieSim {

public:

  typedef vector<Individual> Population;


  // Ctor and Dtor
  GillespieSim(const size_t popSize, gsl_rng* rng); // Constructor for random I1
  ~GillespieSim(); // Destructor


  // Data functions
  void loadCovariates(const string dataPrefix); // Loads in the covariates
  void loadEpiData(const string filename, const double obsTime, const double a, const double b, const double c);
  void loadCTData(const string filename);
  void setIndexCase(const size_t indexCase, const double I, const double N, const double R);
  size_t getIndexCase();
  void setStartTime(const double startTime = 0);
  double getStartTime();
  void setMaxTime(const double maxTime = GSL_POSINF);
  void setMaxN(const size_t maxNumberOfNotifications);
  double getMaxTime();
  void loadDCData(const string filename);
  void addInfection(const size_t label, const double I);


  // Run the simulation
  void simulate(vector<double>& myBeta,
                const double my_a,
                const double my_b,
                const double my_c); // Run model


  // Reset the simulation
  void resetPopulation();
  void resetEpidemic();
  void resetCTData();


  size_t totalSize();
  void writeSimToFile(const string filename,
                      const bool includeCensored = false,
                      const bool includeDC = false) const; // Writes model output to a file
  void writeCTToFile(const string filename, const bool censored = false);
  bool isEpidemicOver();  // Returns true if the epidemic is over at the end of the simulation


  // Bit of maths

  double rng_extreme(double a,double b);



private:

  // XML bits
  XmlCTWriter* contactWriter;
  typedef map<int,XmlCTData*> ContactData;
  ContactData contactData;

  bool ctOutput;


  class frequencies {
  public:
    freq_t fm;
    float fm_N;
    freq_t sh;
    float sh_N;
  };

  SpeciesMatrix species;  


  gsl_rng *rng;        // Random number generator
  int rv;              // Generic return value
  double curr_time;     // The current time in the simulation
  bool init_done;      // Boolean to make sure we've called init before run

  const size_t NPARMS;

  ofstream hFuncOut;

  size_t N_total;     // Total population size
  float *rho;         // Euclidean distance matrix
  float *beta_ij;     // Transmission parms for I(i) -> S(j)
  float *betastar_ij; // Transmission parms for N(i) -> S(j)
  double sum_beta;     // The sum of the transmission rates

  contactMat fm_Mat,sh_Mat,cp_Mat;
  vector<frequencies> cFreq;

  size_t I1;
  double startTime;
  double maxTime;
  size_t maxN;

  double a,b,c,delta;        // Contains extreme value distn hazard rate and decay rate, time to cull, and kernel decay respectively
  const double f;
  const double g;
  vector<double> beta;  // Contains the betas

  size_t numFMInfecs;
  size_t numFMNonInfecs;
  size_t numSHInfecs;
  size_t numSHNonInfecs;
  size_t numOtherInfecs;


  // Population storage
  Population individuals;
  B_INDEX contactCDFCached;
  B_INDEX contactCDF;  // Susceptibles labels
  B_INDEX infective;    // Infectives notify-time->labels
  B_INDEX notified;      //notify_times; // Perl-like hash of removal-time->label
  int S,R;


  struct result_row {      // Struct to hold a row of simulation output
    double event_time;
    int label;
    char event;
    int S;
    int I;
    int N;
    int R;
  } newRow;

  vector<result_row> result;// Vector of structs (see above) - may require pointers and dynamic memory allocation

  void addResult(EVENTTYPE,Individual*); // Appends a row of results to the output
  int distanceInit(const string filename);
  int freqInit(const string filename);
  void contactCDFInit();
  void execute();

  // Maths methods
  double fmRate(const size_t&, const size_t&);
  double fmInfecRate(const size_t&, const size_t&);
  double shRate(const size_t&, const size_t&);
  double shInfecRate(const size_t&, const size_t&);
  double cpInfecRate(const size_t&, const size_t&);
  double spatialRateI(const size_t&, const size_t&);
  double spatialRateN(const size_t&, const size_t&);
  double speciesj(const size_t& j);

  size_t rng_ztPois(double);
  double betaij(const size_t,const size_t);
  double betaijstar(const size_t,const size_t);
  double hFunc(const double);

  // Stochastic functions
  double getTimeToNextContact();
  Individual* getReceiver();
  Individual* getSender(const Individual* const);
  CONTYPE getContactMethod(const Individual* const, const Individual* const);
  bool isInfectious(const CONTACT&);

  // Population maintenance functions
  void infect(Individual*);
  void notify(Individual* const);
  void remove(Individual*);

  double beta_max();

  void addContact(CONTACT&,bool);
  void publishContacts(Individual* pIndiv, const double startTime);

  const char* conTypeToText(CONTYPE&);

  void getIndividual(size_t);
  void dumpInfective();
  void dumpNotified();
  void dumpCDF();





};







#endif
