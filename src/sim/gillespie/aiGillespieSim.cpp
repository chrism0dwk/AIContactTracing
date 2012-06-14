/* ./src/sim/gillespie/aiGillespieSim.cpp
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
 * aiGillespieSim.cpp
 *
 *  Created on: 22 Feb 2010
 *      Author: stsiab
 */

#include <iostream>
#include <vector>
#include <map>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <boost/foreach.hpp>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <gsl/gsl_rng.h>

#include "GillespieSim.hpp"

typedef map<string, double> ParmMap;

struct Settings
{

  size_t popSize;
  double minTime, maxTime;
  int maxN;
  int I1;
  size_t reps;

  string dataPrefix;
  string epiData;
  string ctData;
  string dcData;
  string outputPrefix;

  bool contactTracing;
  double mu;
  double nu;
  double ctWindowSize;
  double a, b, c;

  double epsilon;
  double p1;
  double p2;
  double beta1;
  double beta2;
  double beta3;
  double psi;
  double eta2;
  double eta3;
  double eta4;
  double eta5;
  double eta6;
  double eta7;
  double eta8;
  double eta9;
  double eta10;

  void
  load(const string& filename)
  {
    using boost::property_tree::ptree;

    typedef boost::property_tree::ptree_bad_data bad_data;
    typedef boost::property_tree::ptree_bad_path bad_xml;
    typedef boost::property_tree::ptree_error runtime_error;
    ptree pt;

    read_xml(filename, pt);

    dataPrefix = pt.get<string> ("aiGillespieSim.paths.dataprefix");
    epiData = pt.get<string> ("aiGillespieSim.paths.epidata", "");
    ctData = pt.get<string> ("aiGillespieSim.paths.ctdata", "");
    dcData = pt.get<string> ("aiGillespieSim.paths.dcdata", "");
    popSize = pt.get<size_t> ("aiGillespieSim.options.popsize");
    minTime = pt.get<double> ("aiGillespieSim.options.mintime", 0);
    maxTime = pt.get<double> ("aiGillespieSim.options.maxtime", GSL_POSINF);
    maxN = pt.get<double> ("aiGillespieSim.options.maxN", GSL_POSINF);
    I1 = pt.get("aiGillespieSim.options.I1", -1);
    reps = pt.get("aiGillespieSim.options.reps", 1);
    outputPrefix = pt.get<string> ("aiGillespieSim.paths.outputprefix");
    contactTracing = pt.get<bool> ("aiGillespieSim.options.contacttracing",
        false);

    mu = pt.get<double> ("aiGillespieSim.constants.mu");
    nu = pt.get<double> ("aiGillespieSim.constants.nu");
    a = pt.get<double> ("aiGillespieSim.constants.a");
    b = pt.get<double> ("aiGillespieSim.constants.b");
    c = pt.get<double> ("aiGillespieSim.constants.c");
    ctWindowSize = pt.get<double> ("aiGillespieSim.constants.ctwindowsize");

    epsilon = pt.get<double> ("aiGillespieSim.parameters.epsilon");
    p1 = pt.get<double> ("aiGillespieSim.parameters.p1");
    p2 = pt.get<double> ("aiGillespieSim.parameters.p2");
    beta1 = pt.get<double> ("aiGillespieSim.parameters.beta1");
    beta2 = pt.get<double> ("aiGillespieSim.parameters.beta2");
    beta3 = pt.get<double> ("aiGillespieSim.parameters.beta3");
    psi = pt.get<double> ("aiGillespieSim.parameters.psi");
    eta2 = pt.get<double> ("aiGillespieSim.parameters.eta2");
    eta3 = pt.get<double> ("aiGillespieSim.parameters.eta3");
    eta4 = pt.get<double> ("aiGillespieSim.parameters.eta4");
    eta5 = pt.get<double> ("aiGillespieSim.parameters.eta5");
    eta6 = pt.get<double> ("aiGillespieSim.parameters.eta6");
    eta7 = pt.get<double> ("aiGillespieSim.parameters.eta7");
    eta8 = pt.get<double> ("aiGillespieSim.parameters.eta8");
    eta9 = pt.get<double> ("aiGillespieSim.parameters.eta9");
    eta10 = pt.get<double> ("aiGillespieSim.parameters.eta10");
  }

};

int
main(int argc, char* argv[])
{

  string configFilename;
  string outputPrefix;
  int seed;

  gsl_rng* rng = gsl_rng_alloc(gsl_rng_mt19937);

  cout << "aiGillespieSim (c) C. Jewell 2010" << endl;

  try
    {
      po::options_description desc("Allowed options");
      desc.add_options()("help,h", "Show help message")
                        ("config,c", po::value<string>(), "config file to use")
                        ("seed,s", po::value<int>(),"random seed")
                        ("output,o", po::value<string>(), "output file prefix");

      po::variables_map vm;
      po::store(po::parse_command_line(argc, argv, desc), vm);
      po::notify(vm);

      if (vm.count("help"))
        {
          cout << desc << "\n";
          return 2;
        }

      if (vm.count("config"))
        {
          configFilename = vm["config"].as<string> ();
        }
      else
        {
          cerr << "Config file required" << "\n";
          cerr << desc << "\n";
          return 2;
        }

      if (vm.count("seed"))
        {
          seed = vm["seed"].as<int> ();
          gsl_rng_set(rng, seed);
        }
      else
        {
          gsl_rng_set(rng, 0);
        }

      if (vm.count("output"))
        {
          outputPrefix = vm["output"].as<string> ();
        }

    }
  catch (exception& e)
    {
      cerr << "Exception: " << e.what() << "\n";
      return 2;
    }
  catch (...)
    {
      cerr << "Unknown exception" << "\n";
      return 2;
    }

  // Set up config
  Settings config;
  try
    {
      config.load(configFilename);
    }
  catch (exception& e)
    {
      cerr << "Loading config failed: " << e.what() << endl;
      return 1;
    }

  // Check if we've over-ridden config file with command line
  // output prefix
  if (outputPrefix.empty())
    outputPrefix = config.outputPrefix;

  // Translate parameters to vector
  vector<double> params;
  params.push_back(config.epsilon);
  params.push_back(config.p1);
  params.push_back(config.p2);
  params.push_back(config.beta1);
  params.push_back(config.beta2);
  params.push_back(config.beta3);
  params.push_back(config.psi);
  params.push_back(config.eta2);
  params.push_back(config.eta3);
  params.push_back(config.eta4);
  params.push_back(config.eta5);
  params.push_back(config.eta6);
  params.push_back(config.eta7);
  params.push_back(config.eta8);
  params.push_back(config.eta9);
  params.push_back(config.eta10);

  GillespieSim* simulation;

  try
    {
      simulation = new GillespieSim(config.popSize, rng);
    }
  catch (exception& e)
    {
      cerr << "Exception occurred initialising GillespieSim.  Error: "
          << e.what() << endl;
      return 2;
    }

  try
    {
      simulation->loadCovariates(config.dataPrefix);
    }
  catch (exception& e)
    {
      cerr << "Exception occurred loading covariates.  Error: " << e.what()
          << endl;
      return 2;
    }

  try
    {
      simulation->setStartTime(config.minTime);
      simulation->setMaxN(config.maxN);
      simulation->setMaxTime(config.maxTime);
    }
  catch (exception& e)
    {
      cerr << "Exception occurred setting minTime, maxN, and maxTime.  Error: "
          << e.what() << endl;
      return 2;
    }

  // Set initial epidemic conditions
  if (!config.epiData.empty())
    {
      try
        {
          simulation->loadEpiData(config.epiData, config.minTime, config.a,
              config.b, config.c);
        }
      catch (exception& e)
        {
          cerr << "Exception occurred loading epi data.  Error: " << e.what()
              << endl;
          return 2;
        }
      if (!config.dcData.empty())
        {
          try
            {
              simulation->loadDCData(config.dcData);
            }
          catch (exception& e)
            {
              cerr << "Exception occurred loading DC data.  Error: "
                  << e.what() << endl;
              return 2;
            }
        }
      if (!config.ctData.empty())
        {
          try
            {
              simulation->loadCTData(config.ctData);
            }
          catch (exception& e)
            {
              cerr << "Exception occurred loading CT data.  Error: "
                  << e.what() << endl;
              return 2;
            }

        }
    }
  else
    {
      double inTime = simulation->rng_extreme(config.a, config.b);

      if (config.I1 != -1)
        simulation->setIndexCase(config.I1, 0.0, inTime, inTime + config.c);

      else
        simulation->setIndexCase(gsl_rng_uniform_int(rng, config.popSize), 0.0,
            inTime, inTime + config.c);
    }

  try
    {
      simulation->simulate(params, config.a, config.b, config.c);
    }
  catch (exception& e)
    {
      cerr << "Exception occurred during simulation.  Error: " << e.what()
          << endl;
      return 2;
    }

  string outputFile;
  outputFile = outputPrefix + ".ipt";
  try
    {
      simulation->writeSimToFile(outputFile);
    }
  catch (exception& e)
    {
      cerr << "Exception occurred writing output.  Error: " << e.what() << endl;
      return 2;
    }

  outputFile = outputPrefix + ".uncensored.ipt";
  try
    {
      simulation->writeSimToFile(outputFile, true, true);
    }
  catch (exception& e)
    {
      cerr << "Exception occurred writing uncensored output.  Error: "
          << e.what() << endl;
      return 2;
    }

  outputFile = outputPrefix + ".contact.xml";
  try
    {
      simulation->writeCTToFile(outputFile);
    }
  catch (exception& e)
    {
      cerr << "Exception occurred writing contact xml.  Error: " << e.what()
          << endl;
      return 2;
    }

  outputFile = outputPrefix + ".uncensored.contact.xml";
  try
    {
      simulation->writeCTToFile(outputFile, true);
    }
  catch (exception& e)
    {
      cerr << "Exception occurred writing uncensored contact xml.  Error: "
          << e.what() << endl;
      return 2;
    }

  if (simulation->isEpidemicOver())
    return 0;
  else
    return 1;

}
