/* ./src/sim/Population.hpp
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
 * Population.hpp
 *
 *  Created on: 11 Dec 2009
 *      Author: Chris Jewell
 */

#ifndef POPULATION_HPP_
#define POPULATION_HPP_

#include <vector>
#include <set>
#include <utility>
#include <string>
#include <iostream>
#include <fstream>
#include <cassert>
#include <gsl/gsl_math.h>

#include "aiTypes.hpp"
#include "Individual.hpp"

using namespace std;

namespace EpiRisk
{

  template<typename Indiv>
    class Population
    {

    private:

      // Private data
      int rv;

      // Private methods
      void
      initContactTracing(const char* const );
      void
      updateInfecMethod();

    public:

      typedef vector<Indiv> PopulationContainer;
      typedef typename PopulationContainer::iterator iterator;
      typedef multiset<iterator> PopulationIndex;
      typedef Indiv indivtype;

      /* data */

      PopulationContainer individuals;
      PopulationIndex infectives;
      PopulationIndex susceptibles;

      double obsTime;
      size_t N_total;
      size_t knownInfections;


      // Ctor & Dtor
      Population();
      Population(const int N_total_, const double obsTime_);
      virtual
      ~Population();
      void resetEventTimes();

      // Data management methods
      void
      loadEpiData(const string epiFilename);
      void
      loadContactTracing(const string filename);
      int
      addInfec(Ilabel_t, eventTime_t, eventTime_t, eventTime_t);
      int
      delInfec(Ipos_t);
      void
      resetOccults(); // Deletes all occults
      void
      clear(); // Clears all infected individuals

      // Data access
      Indiv*
      operator[](const size_t pos);
      iterator begin();
      iterator end();
      size_t size();

      // Time functions
      double
      exposureI(Ipos_t, Ipos_t); // Time for which j is exposed to infected i
      double
      exposureIBeforeCT(Ipos_t, Ipos_t);
      double
      ITimeBeforeCT(Ipos_t);
      double
      exposureN(Ipos_t, Ipos_t); // Time for which j is exposed to notified i
      double
      ITime(Ipos_t); // Time for whichtemplate<typename Individual> i was infective
      double
      NTime(Ipos_t); // Time for which i was notified
      double
      STime(Ipos_t); // Time for which i was susceptible
      iterator
      I1()
      {
        return infectives.first();
      }
      ; // Returns iterator to I1
      iterator
      I2()
      {
        return infectives.first()++;
      }
      ; // Returns iterator to I2


    };

  //////////////////////////////////////////////////////
  ///////////////////// Implementation /////////////////
  //////////////////////////////////////////////////////

  template<typename Indiv>
    Population<Indiv>::Population(const int nTotal_, const double obsTime_) :
      obsTime(obsTime_), N_total(nTotal_), knownInfections(0)
    {
      // Constructor sets the observation time, and initializes the population

      // First set up individuals vector
      for (size_t i = 0; i < nTotal_; ++i)
      {
        individuals.push_back(Indiv(i, obsTime, obsTime, obsTime));
      }

    }

  template<typename Indiv>
    Population<Indiv>::Population() : obsTime(GSL_POSINF), N_total(0), knownInfections(0)
    {

    }

  template<typename Indiv>
    Population<Indiv>::~Population()
    {

    }

  template<typename Indiv>
    void
    Population<Indiv>::resetEventTimes()
    {
    for (size_t i = 0; i < N_total; ++i)
      {
        individuals[i].I = obsTime;
        individuals[i].N = obsTime;
        individuals[i].R = obsTime;
      }
    }

  template<typename Indiv>
    Indiv*
    Population<Indiv>::operator[](const size_t pos)
    {
      // Returns a pointer (looked up in individuals) to an individual
      assert(pos < individuals.size()); // Range check
      return &individuals[pos];
    }

  template<typename Indiv>
    typename Population<Indiv>::iterator
    Population<Indiv>::begin()
    {
      return individuals.begin();
    }

  template<typename Indiv>
    typename Population<Indiv>::iterator
    Population<Indiv>::end()
    {
      return individuals.end();
    }

  template<typename Indiv>
    size_t
    Population<Indiv>::size()
    {
      return N_total;
    }

  template<typename Indiv>
    void
    Population<Indiv>::loadEpiData(const string filename)
    {
      //! Loads epidemic data from a .ipt file

      // Populate individuals with infection data and create infected index

      Indiv tempInfection(0, 0, 0, 0);
      ifstream datafile;
      char line[100];
      pair<typename PopulationIndex::iterator, bool> retVal;

      cout << "Reading epidemic file '" << filename.c_str() << "'..." << flush;
      datafile.open(filename.c_str(), ios::in);
      if (!datafile.is_open())
        {
          throw runtime_error(
              "Cannot open epidemic file. Check path/permissions.");
        }

      knownInfections = 0;
      while (1)
        {
          datafile.getline(line, 200);
          if (datafile.eof())
            break;
          sscanf(line, "%i %lf %lf %lf", &tempInfection.label,
              &tempInfection.I, &tempInfection.N, &tempInfection.R);

          tempInfection.known = 1; // This is a known infection.
          tempInfection.contactStart = tempInfection.N;
          tempInfection.hasBeenInfected = true;
          individuals.at(tempInfection.label) = tempInfection;
          infectives.insert(individuals.end() - 1);
        }

      knownInfections = infectives.size();
      datafile.close();

      // Set up susceptibles
      for (iterator iter = individuals.begin(); iter
          != individuals.end(); iter++)
        {
          if (!iter->hasBeenInfected)
            {
              susceptibles.insert(iter);
            }
        }

      cout << "Done" << endl;
    }

  template<typename Indiv>
    void
    Population<Indiv>::loadContactTracing(const string filename)
    {
      // Function associates infections with CT data
      SAXContactParse(filename, individuals);
      updateInfecMethod();
    }

  template<typename Indiv>
    void
    Population<Indiv>::updateInfecMethod()
    {
      // Goes through infectives and finds if infected
      // by a contact or not.

      for (typename PopulationIndex::iterator iter = infectives.begin(); iter
          != infectives.end(); iter++)
        {
          (*infectives)->updateInfecByContact();
        }
    }

  template<typename Indiv>
    int
    Population<Indiv>::addInfec(Spos_t susc_pos, eventTime_t thisI,
        eventTime_t thisN, eventTime_t thisR)
    {

      susceptibles.at(susc_pos)->I = thisI;
      susceptibles.at(susc_pos)->status = Indiv::INFECTED;
      infectives.push_back(susceptibles.at(susc_pos));
      susceptibles.erase(susceptibles.begin() + susc_pos);
      return (0);
    }

  template<typename Indiv>
    int
    Population<Indiv>::delInfec(Ipos_t infec_pos)
    {
      if (infectives.at(infec_pos)->known == 1)
        throw logic_error("Deleting known infection");

      susceptibles.push_back(infectives.at(infec_pos));
      susceptibles.back()->status = Indiv::SUSCEPTIBLE;
      susceptibles.back()->I = susceptibles.back()->N;
      infectives.erase(infectives.begin() + infec_pos);
      return (0);
    }

  template<typename Indiv>
    double
    Population<Indiv>::exposureI(Ipos_t i, Ipos_t j)
    {
      // NB: This gives time that susceptible j is exposed to infective i before becoming infected
      return GSL_MIN(individuals[i].N, individuals[j].I) - GSL_MIN(
          individuals[i].I, individuals[j].I);
    }

  template<typename Indiv>
    double
    Population<Indiv>::exposureIBeforeCT(Ipos_t i, Ipos_t j)
    {
      // Returns time that susceptible j is exposed to infective i before
      // either its contact tracing started or it got infected (the latter in the
      // unlikely event that it was infected before the contact tracing started)

      double stopTime;
      double startTime;
      double earliestContactStart = GSL_MIN(individuals[i].contactStart,
          individuals[j].contactStart);

      stopTime = GSL_MIN(individuals[i].N, individuals[j].I);
      stopTime = GSL_MIN(earliestContactStart, stopTime);

      startTime = GSL_MIN(individuals[j].I, individuals[i].I);
      startTime = GSL_MIN(earliestContactStart, stopTime);

      return stopTime - startTime;
    }

  template<typename Indiv>
    double
    Population<Indiv>::ITimeBeforeCT(Ipos_t i)
    {
      // Returns the amount of time between I and start of CT window
      // Non-neg if CTstart > I, 0 otherwise

      double iTime = individuals.at(i).contactStart - individuals[i].I;

      if (iTime > 0)
        return iTime;
      else
        return 0.0;
    }

  template<typename Indiv>
    double
    Population<Indiv>::exposureN(Ipos_t i, Ipos_t j)
    {
      // NB: This gives time that susceptible j is exposed to notified i before becoming infected
      return GSL_MIN(individuals[i].R, individuals[j].I) - GSL_MIN(
          individuals[j].I, individuals[i].N);
    }

  template<typename Indiv>
    double
    Population<Indiv>::ITime(Ipos_t i)
    {
      // NB: Gives the time for which i was infectious but not notified
      return individuals.at(i).N - individuals[i].I;
    }

  template<typename Indiv>
    double
    Population<Indiv>::NTime(Ipos_t i)
    {
      // NB: Gives the time for which i was notified
      return individuals[i].R - individuals[i].N;
    }

  template<typename Indiv>
    double
    Population<Indiv>::STime(Ipos_t i)
    {
      // NB: Gives the time for which i was susceptible
      return individuals[i].I - individuals[I1].I;
    }

}
#endif /* POPULATION_HPP_ */
