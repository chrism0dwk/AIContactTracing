/* ./src/data/sinrEpi.cpp
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


/* sinrEpi class definitions for SINR Epidemic */

#include "sinrEpi.h"
#include "EpiRiskException.hpp"


sinrEpi::sinrEpi() : rho(NULL) {
}



sinrEpi::~sinrEpi() {
  if(rho != NULL) delete rho;
}



int sinrEpi::freqInit(const char *filename) {

  ifstream datafile;
  char line[200];
  frequencies freqRow;
  int i;

  cFreq.clear();

  //#pragma omp parallel for default(shared) private(i) schedule(static)
  //for(i=0; i < N_total; ++i) cFreq.push_back(freqRow);

  datafile.open(filename,ios::in);
  if(!datafile.is_open()) {
    throw EpiRisk::data_exception("frequency file");
  }

  for(i = 0; i < N_total; ++i) {
    datafile.getline(line,200);
    if(datafile.eof()) break;
    sscanf(line,"%f %f %f %f",&freqRow.fm,&freqRow.fm_N,&freqRow.sh,&freqRow.sh_N);
    cFreq.push_back(freqRow);
  }

  datafile.close();

  return(0);
}



int sinrEpi::init(const size_t myN_total,
        const char *epiFile,
        const char *contactPrefix,
        const char *distFile,
        const size_t nSpecies,
        const double _obsTime) {

  N_total = myN_total;
  obsTime = _obsTime;

  /* Set up the epidemic */

  I1 = 0; // Start with defining the first infection at the beginning of the infected vector
  ifstream datafile;
  char filename[200];
  char line[200];
  infection tempInfection(0,0,0,0,0,SUSCEPTIBLE);

  // Start with all susceptible
  for(int i=0; i<N_total; ++i) {
    individuals.push_back(infection(i,
				    obsTime,
				    obsTime,
				    obsTime,
				    0,
				    SUSCEPTIBLE));
  }

  // Populate individuals with infection data and create infected index
  sprintf(filename,"%s.ipt",epiFile);
  datafile.open(filename,ios::in);
  if(!datafile.is_open()) {
    throw EpiRisk::data_exception("opening .ipt file");
  }
  cout << "Parsing epi file..." << endl;

  while(1) {
    datafile.getline(line,200);
    if(datafile.eof()) break;
    sscanf(line,"%i %lf %lf %lf",&tempInfection.label,&tempInfection.I,&tempInfection.N,&tempInfection.R);

    tempInfection.known = 1; // This is a known infection.
    tempInfection.contactStart = tempInfection.N;
    tempInfection.status = INFECTED;
    individuals.at(tempInfection.label) = tempInfection;
    infected.push_back(&(individuals.at(tempInfection.label)));
  }
  datafile.close();


 // Population individuals with DC data
  sprintf(filename,"%s.dc",epiFile);
  datafile.open(filename,ios::in);
  if(!datafile.is_open()) {
    throw EpiRisk::data_exception("opening .dc file");
  }

  while(1) {
    datafile.getline(line,200);
    if(datafile.eof()) break;
    sscanf(line,"%i %lf %lf", &tempInfection.label,&tempInfection.N,&tempInfection.R);
    individuals.at(tempInfection.label).isDC = true;
    individuals.at(tempInfection.label).known = true;
    individuals.at(tempInfection.label).I = tempInfection.N - 0.5;
    individuals.at(tempInfection.label).N = tempInfection.N;
    individuals.at(tempInfection.label).R = tempInfection.R;
    individuals.at(tempInfection.label).status = INFECTED;
    infected.push_back(&(individuals.at(tempInfection.label)));
  }
  datafile.close();


  // Population individuals with NI data
  sprintf(filename,"%s.ni",epiFile);
  datafile.open(filename,ios::in);
  if(!datafile.is_open()) {
    throw EpiRisk::data_exception("opening .ni file");
  }

  while(1) {
    datafile.getline(line,200);
    if(datafile.eof()) break;
    sscanf(line,"%i %lf",&tempInfection.label,&tempInfection.N);
    individuals.at(tempInfection.label).niAt = tempInfection.N;
  }
  datafile.close();



  // Now set up the susceptible index
  vector< infection >::iterator indiv_iter = individuals.begin();

  while(indiv_iter < individuals.end()) {
    if(indiv_iter->status == SUSCEPTIBLE) {
      susceptible.push_back(&(*indiv_iter));
    }
    indiv_iter++;
  }

  knownInfections = infected.size();  // This is where we store the total KNOWN infectives (ie those that have been notified) before we start imputing infections.

  cout << "Epidemic initialised!" << endl;


    // Read in contact tracing data

    sprintf(filename,"%s.contact.xml",epiFile);
    cout << "Reading contact data from " << filename << endl;
    try {
     initContactTracing(filename);
    }
    catch (exception& e) {
     cerr << "Exception occurred:\n\t" << e.what() << endl;
     throw logic_error("Dodgy contact tracing data.");
    }

    cout << "Done" << endl;


  // Update infection method indicator

  cout << "Updating infection method indicators...";
  updateInfecMethod();
  cout << "Done" << endl;



  /* Set up the contact matrix */
  sprintf(filename,"%s.fm",contactPrefix);
  cout << "Reading feedmill matrix from " << filename << "...";
  rv = fm_Mat.init(filename,N_total);
  if(rv != 0) {
    cout << "Feed Mill Contact Matrix could not be initialised!" << endl;
    return(-1);
  }
  cout << "Done" << endl;

  sprintf(filename,"%s.sh",contactPrefix);
  cout << "Reading slaughterhouse contact matrix from " << filename << "...";
  rv = sh_Mat.init(filename,N_total);
  if(rv != 0) {
    cout << "SH Contact Matrix could not be initialised!" << endl;
    return(-1);
  }
  cout << "Done" << endl;

  sprintf(filename,"%s.cp",contactPrefix);
  cout << "Reading company contact matrix from " << filename << "...";
  rv = cp_Mat.init(filename,N_total);
  if(rv != 0) {
    cout << "Company Contact Matrix could not be initialised!" << endl;
    return(-1);
  }
  cout << "Done" << endl;

  sprintf(filename,"%s.freq",contactPrefix);
  cout << "Reading contact frequency data from " << filename << "...";
  rv = freqInit(filename);
  if(rv != 0) {
    cout << "Frequency table could not be loaded!" << endl;
    return(-1);
  }
  cout << "Done" << endl;

  cout << "Contact matrices initialised!" << endl;

  sprintf(filename,"%s.sp",contactPrefix);
  rv = species.initialize(filename,N_total,nSpecies);
  if(rv !=0) {
    cout << "Species table could not be initialized!" << endl;
    return(-1);
  }

  cout << "Species matrix initialised!" << endl;



  /* Set up the distance matrix */

  rho = new (nothrow) float[N_total*N_total];
  if (rho == 0) {
    cout << "Can't allocate rho!" << endl;
    return(-1);
  }
  rv = rhoInit(distFile,rho);
  if(rv != 0) {
    cout << "Initialisation of rho failed!" << endl;
    return(-1);
  }

  return(0);

}



int sinrEpi::rhoInit(const char *filename,float *myRho)
{

  ifstream datafile;
  int i,j;
  float dist;
  char line[200];

  // Set all elements of rho to GSL_POS_INF:
#pragma omp parallel for default(shared) private(i,j) schedule(static)
  for(int i=0; i < N_total;++i) {
    for(int j=0; j < N_total; ++j) {
      *(myRho+i+N_total*j) = GSL_POSINF;
    }
  }


  datafile.open(filename,ios::in);
  if(!datafile.is_open()) {
    cout << "Cannot open distance file" << filename << endl;
    return(-1);
  }

  while(1) {

    datafile.getline(line,200);
    if(datafile.eof()) break;
    if(strlen(line) <= 1) break;
    sscanf(line,"%i %i %f",&i,&j,&dist);
    *(myRho + i + N_total*j) = dist;
  }

  datafile.close();

  return(0);
}



void sinrEpi::initContactTracing(const char* const filename)
{
  // Function associates infections with CT data
  cout << "Starting parse" << endl;
  SAXContactParse(filename,individuals);

//   // Sort contacts
//   vector<infection>::iterator it = individuals.begin();
//   while(it != individuals.end()) {
//     it->contacts.sort();
//     it++;
//   }

  cout << "Finished parse, handing back to sinrEpi" << endl;
}


void sinrEpi::updateInfecMethod()
{
  // Goes through infectives and finds if infected
  // by a contact or not.

  for(int i=0; i<infected.size(); ++i) {
    infected[i]->updateInfecByContact();
  }
}



int sinrEpi::addInfec(Spos_t susc_pos, eventTime_t thisI, eventTime_t thisN, eventTime_t thisR) {

  susceptible.at(susc_pos)->I=thisI;
  //susceptible.at(susc_pos)->N=thisN;
  //susceptible.at(susc_pos)->R=thisR;
  //susceptible.at(susc_pos)->contactStart = thisN;
  //susceptible.at(susc_pos)->known=0;
  susceptible.at(susc_pos)->status = INFECTED;
  infected.push_back(susceptible.at(susc_pos));
  susceptible.erase(susceptible.begin()+susc_pos);
  return(0);
}



int sinrEpi::delInfec(Ipos_t infec_pos)
{
  if(infected.at(infec_pos)->known == 1)
    throw logic_error("Deleting known infection");

  susceptible.push_back(infected.at(infec_pos));
  susceptible.back()->status = SUSCEPTIBLE;
  susceptible.back()->I = susceptible.back()->N;
  infected.erase(infected.begin()+infec_pos);
  return(0);
}





double sinrEpi::exposureI(Ipos_t i, Ipos_t j)
{
  // NB: This gives time that susceptible j is exposed to infective i before becoming infected
  return GSL_MIN(individuals[i].N,individuals[j].I) - GSL_MIN(individuals[i].I,individuals[j].I);
}



double sinrEpi::exposureIBeforeCT(Ipos_t i, Ipos_t j)
{
  // Returns time that susceptible j is exposed to infective i before
  // either its contact tracing started or it got infected (the latter in the
  // unlikely event that it was infected before the contact tracing started)
 
  double stopTime;
  double startTime;
  double earliestContactStart = GSL_MIN(individuals[i].contactStart,
					individuals[j].contactStart);

  stopTime = GSL_MIN(individuals[i].N,individuals[j].I);
  stopTime = GSL_MIN(earliestContactStart,stopTime);

  startTime = GSL_MIN(individuals[j].I,individuals[i].I);
  startTime = GSL_MIN(earliestContactStart,stopTime);

  return stopTime - startTime;
}



double sinrEpi::ITimeBeforeCT(Ipos_t i)
{
  // Returns the amount of time between I and start of CT window
  // Non-neg if CTstart > I, 0 otherwise

  double iTime = individuals.at(i).contactStart - individuals[i].I;

  if(iTime > 0) return iTime;
  else return 0.0;
}


double sinrEpi::exposureN(Ipos_t i, Ipos_t j)
{
  // NB: This gives time that susceptible j is exposed to notified i before becoming infected
  return GSL_MIN(individuals[i].R,individuals[j].I) - GSL_MIN(individuals[j].I,individuals[i].N);
}



double sinrEpi::ITime(Ipos_t i) 
{
  // NB: Gives the time for which i was infectious but not notified
  return individuals.at(i).N - individuals[i].I;
}



double sinrEpi::NTime(Ipos_t i) {
  // NB: Gives the time for which i was notified
  return individuals[i].R - individuals[i].N;
}



double sinrEpi::STime(Ipos_t i)
{
  // NB: Gives the time for which i was susceptible
  return individuals[i].I - infected[I1]->I;
}



Ipos_t sinrEpi::updateI1()
{
  /* Evaluate initial infection */

  Ipos_t myI1_index = 0;

  for(unsigned int h=0; h < infected.size(); ++h) {
    if(infected[h]->I < infected[myI1_index]->I) {
      myI1_index = h;
    }
  }

  I1 = myI1_index;
  return(myI1_index);
}



Ipos_t sinrEpi::I2()
{
  /* Returns the index of I2 in vector<infection> infected */

  Ipos_t startPos, myI2;

  if(I1 == 0) startPos = 1;
  else startPos = 0;

  myI2 = startPos;

  for(Ipos_t h=startPos; h < infected.size(); ++h) {

    if(h == I1) continue; // Skip I1

    if(infected[h]->I < infected[myI2]->I)
      myI2 = h;
  }

  return(myI2);
}







sinrEpi::frequencies::frequencies() :
  fm(0.0),
  fm_N(0),
  sh(0.0),
  sh_N(0)
{
}



sinrEpi::frequencies::frequencies(const freq_t a, const int b, const freq_t c, const int d) :
  fm(a),
  fm_N(b),
  sh(c),
  sh_N(d)
{
}



double sinrEpi::sumI()
{
  // Add up all the infection times excluding I1

  double sumI = 0.0;
  for(size_t i = 0; i<infected.size(); ++i) {
    if(i != I1) sumI += infected[i]->I;
  }
  return sumI;
}



double sinrEpi::mean_I()
{
  // Return the mean of the 'known' infection times

  double mean_I=0;
  for(size_t z=0; z < knownInfections; ++z) {
    mean_I += (infected[z]->N - infected[z]->I);
  }
  return mean_I / knownInfections;
}



size_t sinrEpi::numAdditions()
{
  return infected.size() - knownInfections;
}



void sinrEpi::resetOccults()
{
    // Deletes all occults

//    infected.erase(infected.begin() + knownInfections,infected.end());
    
    while(infected.size() > knownInfections)
      delInfec(infected.size()-1);
    
}



void sinrEpi::dumpEpidemic()
{
  // Dumps the epidemic to stdout in .ipt format
  vector<infection>::iterator iter = individuals.begin();
  while(iter != individuals.end()) {
    cout << iter->label << " " << iter->I << " " << iter->N << " " << iter-> R << endl;
    iter++;
  }
}


epiParms::epiParms(const int myP) : p(myP)
{

  try{
    beta = new double[p];
    }
  catch(bad_alloc&) {
    cerr << "Could not allocate beta!" << endl;
  }
}

epiParms::~epiParms()
{

  delete[] beta;
}

int epiParms::operator=(epiParms &b) {

  for(int i=0; i < p; ++i) {
    beta[i] = b.beta[i];
  }
  return(0);
}



bool epiParms::isBetaNegative()
{
  //Checks to see >= 1 component of beta is negative

  for(size_t k=0;k<p;++k) {
    if(beta[k] < 0) {
      return 1;
    }
  }

  return 0;
}



epiPriors::epiPriors(int p)
{

  try{
    lambda = new double[p];
    nu = new double[p];
    }
    catch(bad_alloc&) {
    cerr << "Could not allocate priors!" << endl;
    }
}



epiPriors::~epiPriors()
{

  delete[] lambda;
  delete[] nu;
}



