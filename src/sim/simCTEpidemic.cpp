/* ./src/sim/simCTEpidemic.cpp
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
 * simCTEpidemic.cpp
 *
 *  Created on: 5 Jan 2010
 *      Author: stsiab
 *     Purpose: Simulates an epidemic on top of a dynamic contact network
 */


#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <gsl/gsl_math.h>

#include "SimOnContact.hpp"

using namespace std;


struct Settings
{
  string contactsFile;
  string dcDataFile;
  string thresholdPrefix;
  string inTimesFile;
  double nrTime;
  size_t popSize;
  double maxTime;
  int I1;
  size_t reps;
  string outputPrefix;
  bool contactTracing;

  double mu;
  double nu;
  double ctWindowSize;
  double p1;
  double p2;

  void load(const string& filename) {
    using boost::property_tree::ptree;

    typedef boost::property_tree::ptree_bad_data bad_data;
    typedef boost::property_tree::ptree_bad_path bad_xml;
    typedef boost::property_tree::ptree_error runtime_error;
    ptree pt;

    read_xml(filename, pt);
    contactsFile = pt.get<string>("simOnContacts.contactFile");
    dcDataFile = pt.get("simOnContacts.dcdatafile","");
    thresholdPrefix = pt.get<string>("simOnContacts.thresholdPrefix");
    inTimesFile = pt.get<string>("simOnContacts.inTimes");
    nrTime = pt.get<double>("simOnContacts.nrTime");
    popSize = pt.get<size_t>("simOnContacts.popSize");
    maxTime = pt.get<double>("simOnContacts.maxTime",GSL_POSINF);
    I1 = pt.get("simOnContacts.I1",-1);
    reps = pt.get("simOnContacts.reps",1);
    outputPrefix = pt.get<string>("simOnContacts.outputPrefix");
    contactTracing = pt.get<bool>("simOnContacts.contactTracing",false);

    mu = pt.get<double>("simOnContacts.constants.mu");
    nu = pt.get<double>("simOnContacts.constants.nu");
    ctWindowSize = pt.get<double>("simOnContacts.constants.ctWindowSize");
    p1 = pt.get<double>("simOnContacts.constants.p1");
    p2 = pt.get<double>("simOnContacts.constants.p2");
  }

};



int main(int argc, char* argv[])
{
  if (argc != 2) {
    cerr << "Usage: simCTEpidemic <config file>" << endl;
    return -1;
  }

  // Get settings
  Settings config;

  try {
    config.load(argv[1]);
  }
  catch (boost::property_tree::ptree_bad_data& e) {
    cerr << "Bad data in config file: " << e.what() << endl;
    return -1;
  }
  catch (boost::property_tree::ptree_bad_path& e) {
    cerr << "Malformed XML in config file: " << e.what() << endl;
    return -1;
  }
  catch (boost::property_tree::ptree_error& e) {
    cerr << "Cannot load config file: " << e.what() << endl;
    return -1;
  }
  catch (...) {
    cerr << "Unknown error parsing config file" << endl;
    return -1;
  }


  // Create simulation
  cerr << "Initialising..." << flush;
  SimOnContact* simulation = new SimOnContact(config.contactsFile,config.popSize);
  cerr << "Done" << endl;

  cerr << "Loading thresholds..." << flush;
  simulation->loadThresholds(config.thresholdPrefix);
  cerr << "Done" << endl;
  cerr << "Loading inTimes" << flush;
  simulation->loadInTimes(config.inTimesFile);
  cerr << "Done" << endl;

  simulation->setCtWindowSize(21.0);
  simulation->setHFuncParms(config.mu,config.nu);
  simulation->setMaxTime(config.maxTime);
  simulation->setParms(config.p1,config.p2);

  if(!config.dcDataFile.empty()) simulation->loadDCData(config.dcDataFile);

  // Seed the PRNG
  if(config.I1 == -1) srand(time(NULL));

  // Perform simulation
  int I1; // Our initial infective
  stringstream outputPrefix;

  for(size_t i = 0; i < config.reps ; ++i) {
    cerr << "Simulation " << i << "..." << flush;

    // If we have a -ve I1 cmdline parameter, choose a random index case.
    if (config.I1 == -1) {
      I1 = rand() % config.popSize;
    }
    else I1 = config.I1;


    simulation->reset();
    simulation->addInfection(I1);
    simulation->simulate();

    // Write normal .ipt file
    outputPrefix.str("");
    outputPrefix << config.outputPrefix << ".ipt";
    simulation->writeSimToFile(outputPrefix.str(),false,false);

    // Write uncensored .ipt file
    outputPrefix.str("");
    outputPrefix << config.outputPrefix << ".uncensored.ipt";
    simulation->writeSimToFile(outputPrefix.str(),true,true);

    // Write contact tracing
    if (config.contactTracing) {
      outputPrefix.str("");
      outputPrefix << config.outputPrefix << ".contact.xml";
      simulation->writeCTToFile(outputPrefix.str());
    }

    cerr << "Done" << endl;

  }


  delete simulation;


  return 0;
}


