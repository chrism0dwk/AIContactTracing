/* ./src/sim/SimOnContact.hpp
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
 * SimOnContact.hpp
 *
 *  Created on: 21 Jan 2010
 *      Author: stsiab
 */

#ifndef SIMONCONTACT_HPP_
#define SIMONCONTACT_HPP_

#include <cstdlib>
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <iterator>
#include <memory>
#include <string>

#include <sstream>
#include <fstream>
#include <iomanip>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <gsl/gsl_math.h>

#include "EpiRiskException.hpp"
#include "Population.hpp"
#include "EventQueue.hpp"
#include "XmlCTWriter.hpp"

using namespace std;
using namespace EpiRisk;



class ThresholdMatrix
{
public:
  ThresholdMatrix(const string filename, const size_t nTotal);
  ~ThresholdMatrix();

  double get(const int i, const int j);

private:
  float* th_;
  size_t nTotal_;
};


// Comparison functor

class CompInfectionTime
{
public:
  bool operator()(const Individual& lhs, const Individual& rhs) const
    { return lhs.I < rhs.I; }
};



// InfectionData typdef
typedef std::set<Individual,CompInfectionTime> InfectionData;



class SimOnContact
{
public:
  SimOnContact(const string filename, const size_t nTotal);
  virtual
  ~SimOnContact();
  void addInfection(const Individual& indiv);
  void addInfection(const size_t label);
  void simulate();
  void reset();

  // Load data
  void loadContactData(const string filename);
  void loadThresholds(const string filePrefix);
  void loadInTimes(const string filename);
  void loadDCData(const string filename);
  void loadOccults(const string filename);

  // Getters and setters
  void setMaxTime(const double maxTime);
  double getMaxTime() const;

  void setParms(const double p1, const double p2);
  void getParms(double* p1, double* p2);

  void setHFuncParms(const double Mu, const double Nu);
  void getHFuncParms(double* Mu, double* Nu);

  void setCtWindowSize(const double CTWindowSize);
  double getCtWindowSize() const;


  // Write data
  void writeSimToFile(const string filePrefix, const bool includeCensored = false, const bool includeDC = false) const;
  void writeCTToFile(const string filename) const;


private:

  static bool compInfectionTime(const Individual& lhs, const Individual& rhs);

  class CompNotificationTimePtr
  {
  public:
    bool operator()(Individual* lhs, Individual* rhs) const;
  };

  void infect(const size_t label);
  void notify(const size_t label);
  void remove(const size_t label);
  void appendContact(size_t& contacteeId, size_t& contactorId, const char* type, const double& time, bool caused);
  double hFunc(const double t);

  typedef EpiRisk::Population<EpiRisk::Individual> Population;
  typedef multiset<EpiRisk::Individual*, CompNotificationTimePtr> NotificationSet;
  typedef map<int, XmlCTData* > ContactData;

  struct DCTimes {
    double N;
    double R;
  };

  typedef map<size_t, DCTimes> DCData;

  // Population state
  Population* individuals;
  NotificationSet notifications;
  size_t numS, numI, numN, numR;
  double currTime;


  // Input data
  ContactQueue* contactQueue;
  ThresholdMatrix* hFuncTh;
  ThresholdMatrix* fmThres;
  ThresholdMatrix* shThres;
  double* inTimes;
  double nrTime;
  DCData dcData;

  // Output data
  XmlCTWriter* ctWriter;
  ContactData contactData;

  // Constants
  string epiDataPrefix;
  size_t nTotal;
  double ctWindowSize;
  double maxTime, minTime;
  double P1, P2;
  double mu, nu;




  enum EventType { INFECTION=0,
                   NOTIFICATION,
                   REMOVAL
  };
};

#endif /* SIMONCONTACT_HPP_ */
