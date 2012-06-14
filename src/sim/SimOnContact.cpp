/* ./src/sim/SimOnContact.cpp
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
 * SimOnContact.cpp
 *
 *  Created on: 21 Jan 2010
 *      Author: stsiab
 */

#include <algorithm>

#include <gsl/gsl_math.h>

#include "SimOnContact.hpp"
#include "stlStrTok.hpp"

//  Individual::Individual(const double label_, const double i, const double n,
//      const double r) :
//    I(i), N(n), R(r), label(label_)
//  {
//  }
//
//  Individual::Status
//  Individual::status(const double time)
//  {
//    if (time <= I)
//      return SUSCEPTIBLE;
//    else if (I < time && time <= N)
//      return INFECTED;
//    else if (N < time && time <= R)
//      return NOTIFIED;
//    else
//      return REMOVED;
//  }
//
//  bool
//  Individual::isSusceptibleAt(const double time) const
//  {
//    return time <= I;
//  }
//
//  bool
//  Individual::isInfectedAt(const double time) const
//  {
//    return I < time && time <= N;
//  }
//
//  bool
//  Individual::isNotifiedAt(const double time) const
//  {
//    return N < time && time <= R;
//  }
//
//  bool
//  Individual::isRemovedAt(const double time) const
//  {
//    return R < time;
//  }



ThresholdMatrix::ThresholdMatrix(const string filename, const size_t nTotal) :
  th_(NULL), nTotal_(nTotal)
{

  th_ = new float[nTotal * nTotal];

  // Initialise hFuncSeeds
  ifstream file;
  string buff, word;
  stringstream msg;
  file.open(filename.c_str(), ios::in);
  if (!file.is_open())
    {
      msg << "Cannot open hFuncFile '" << filename.c_str() << "'.";
      throw runtime_error(msg.str().c_str());
    }

  for (size_t i = 0; i < nTotal; i++)
    {
      getline(file, buff);
      istringstream buffstream(buff, ios::in);
      for (size_t j = 0; j < nTotal; j++)
        {
          getline(buffstream, word, ' ');
          if (word.length() < 2)
            {
              cerr << "Wrong dimension in hFunc seed file.  Check it." << endl;
              abort();
            }
          *(th_ + i * nTotal + j) = atof(word.c_str());
        }
    }

  file.close();

}

ThresholdMatrix::~ThresholdMatrix()
{
  if (th_)
    delete[] th_;
}

double
ThresholdMatrix::get(const int i, const int j)
{
  return *(th_ + nTotal_ * i + j);
}

/////////////////////////////////////
///// MAIN CLASS IMPLEMENTATION /////
/////////////////////////////////////

SimOnContact::SimOnContact(const string contactDataFile, const size_t nTotal) :
  numS(nTotal), numI(0), numN(0), numR(0), currTime(0.0), hFuncTh(NULL),
      fmThres(NULL), shThres(NULL), inTimes(NULL), nrTime(1.0), ctWriter(NULL),
      nTotal(nTotal), ctWindowSize(21.0), maxTime(GSL_POSINF), minTime(0.0),
      P1(0.5), P2(0.5), mu(0), nu(0)
{

  // Set up contact queue
  contactQueue = new ContactQueue(contactDataFile);
  ctWriter = 0;

  // Set up population
  individuals = new Population(nTotal, maxTime);


}

SimOnContact::~SimOnContact()
{
  if (ctWriter)
    delete ctWriter;
  if (hFuncTh)
    delete hFuncTh;
  if (fmThres)
    delete fmThres;
  if (shThres)
    delete shThres;
  if (inTimes)
    delete[] inTimes;
  if (contactQueue)
    delete contactQueue;

  delete individuals;
}

void
SimOnContact::setMaxTime(const double MaxTime)
{
  maxTime = MaxTime;
}

double
SimOnContact::getMaxTime() const
{
  return maxTime;
}

void
SimOnContact::setParms(const double p1, const double p2)
{
  P1 = p1;
  P2 = p2;
}

void
SimOnContact::getParms(double* p1, double* p2)
{
  *p1 = P1;
  *p2 = P2;
}

void
SimOnContact::setHFuncParms(const double Mu, const double Nu)
{
  mu = Mu;
  nu = Nu;
}

void
SimOnContact::getHFuncParms(double* Mu, double* Nu)
{
  *Mu = mu;
  *Nu = nu;
}

void
SimOnContact::setCtWindowSize(const double CTWindowSize)
{
  ctWindowSize = CTWindowSize;
}

double
SimOnContact::getCtWindowSize() const
{
  return ctWindowSize;
}

void
SimOnContact::loadDCData(const string filename)
{
  // Loads Direct Contact cull data
  // File format: <label> <N> <R>

  ifstream file;
  string buffer;
  vector<string> tokens;

  file.open(filename.c_str());
  if(!file.is_open()) throw data_exception("Cannot open DC file");

  dcData.clear();

  while(!file.eof()) {
    getline(file,buffer);
    if(buffer.size() < 2) continue;

    stlStrTok(tokens,buffer," ");
    if (tokens.size() != 3) throw data_exception("Malformed data in DC file");

    double label;
    DCTimes times;

    label = atoi(tokens[0].c_str());
    times.N = atof(tokens[1].c_str());
    times.R = atof(tokens[2].c_str());

    dcData.insert(pair<int,DCTimes>(label,times));

    (*individuals)[label]->isDC = true;
  }

  file.close();
}

void
SimOnContact::loadOccults(const string filename)
{
  // TODO Implement
}

void
SimOnContact::loadInTimes(const string filename)
{
  // Initialise inTimes
  ifstream file;
  string buff;
  stringstream msg;

  if (inTimes)
    delete inTimes;
  inTimes = new double[nTotal];

  file.open(filename.c_str());
  if (!file.is_open())
    {
      msg << "Cannot open inTimes file '" << filename.c_str() << "'";
      throw EpiRisk::data_exception(msg.str().c_str());
    }

  for (size_t i = 0; i < nTotal; ++i)
    {
      getline(file, buff);
      if (buff.length() < 2)
        {
          cerr << "Wrong dimension in inTimes file.  Check it." << endl;
          abort();
        }
      *(inTimes + i) = atof(buff.c_str());
    }

}

void
SimOnContact::loadThresholds(const string filePrefix)
{
  // Initialise Thresholds

  stringstream filename;

  // hFunc
  if (hFuncTh)
    delete hFuncTh;
  filename.str("");
  filename << filePrefix << ".hFunc";
  hFuncTh = new ThresholdMatrix(filename.str(), nTotal);

  // Initialise fm thresholds
  if (fmThres)
    delete fmThres;
  filename.str("");
  filename << filePrefix << ".fmTh";
  fmThres = new ThresholdMatrix(filename.str(), nTotal);

  // Initialise sh thresholds
  if (shThres)
    delete shThres;
  filename.str("");
  filename << filePrefix << ".shTh";
  shThres = new ThresholdMatrix(filename.str(), nTotal);

}

void
SimOnContact::addInfection(const Individual& indiv)
{
  // Initialize the epidemic with infections
  cerr << "WARNING: " << __PRETTY_FUNCTION__ << " is incomplete and should not be used" << endl;
  if (indiv.label >= nTotal)
    throw EpiRisk::range_exception("Individual out of range");

  (*individuals)[indiv.label]->I = indiv.I;
  (*individuals)[indiv.label]->N = indiv.N;
  (*individuals)[indiv.label]->R = indiv.R;

}

void
SimOnContact::addInfection(const size_t label)
{
  // Infects an individual as usual
  infect(label);
}

void
SimOnContact::simulate()
{

  // Check existence of data
  if (!hFuncTh)
    throw EpiRisk::data_exception(
        "hFunc threshold data does not exist. Has it been loaded?");
  if (!fmThres)
    throw EpiRisk::data_exception(
        "FM threshold data does not exist. Has it been loaded?");
  if (!shThres)
    throw EpiRisk::data_exception(
        "SH threshold data does not exist. Has it been loaded?");
  if (!inTimes)
    throw EpiRisk::data_exception(
        "I->N time data does not exists.  Has it been loaded?");

  // Simulation loop
  ContactEvent* currEvent = contactQueue->next();
  Individual* contactee;
  Individual* contactor;
  Individual testNotify(0, 0, 0, 0);

  // Contact data
  ContactData::iterator itCTData = contactData.begin();
  while (itCTData != contactData.end())
    {
      delete itCTData->second;
      itCTData++;
    }

  if (ctWriter != NULL)
    delete ctWriter;
  contactData.clear();
  ctWriter = new XmlCTWriter();

  Population::iterator iter = individuals->begin();
  pair<ContactData::iterator, bool> rv;
  while (iter != individuals->end())
    {
      rv = contactData.insert(pair<int, XmlCTData*> (iter->label,
          ctWriter->createCTData(iter->label)));
      if (rv.second == false)
        {
          throw logic_error("Failed to insert contact data");
        }
      iter++;
    }


  // Update times for the DC individuals
  DCData::iterator dcIter = dcData.begin();
  while(dcIter != dcData.end()) {
    (*individuals)[dcIter->first]->I = dcIter->second.N;
    (*individuals)[dcIter->first]->N = dcIter->second.N;
    (*individuals)[dcIter->first]->R = dcIter->second.R;
    dcIter++;
  }



  // Iterate over contact events
  while (numN > 0 && currEvent != NULL)
    {

      currTime = currEvent->time;
      if (currTime > maxTime)
        break;

      contactee = (*individuals)[currEvent->id];

      // Check notifications
      testNotify.N = currTime;
      NotificationSet::iterator upperBound = notifications.upper_bound(
          &testNotify);
      NotificationSet::iterator iter = notifications.begin();
      while (iter != upperBound)
        {
          XmlCTData* data = contactData[(*iter)->label];
          NotificationSet::iterator tempIter = iter;
          data->setCTStartTime((*iter)->N - ctWindowSize);
          data->truncate();
          ctWriter->addCTData(data);
          numN--;
          numR++;
          iter++;
          notifications.erase(tempIter);
        }

      assert(numN >= 0);

      switch (currEvent->type)
        {

      case BACKGROUND:
        if (contactee->isSAt(currTime))
          infect(currEvent->id);
        break;

      case FEEDMILL:
        contactor = (*individuals)[currEvent->from];

        if (contactee->isSAt(currTime))
          {

            if (contactor->isSAt(currTime))
              {
                contactee->addContact(contactor, FEEDMILL, currEvent->time);
              }
            else if (contactor->isIAt(currTime))
              {
                if (P1 > fmThres->get(currEvent->from, currEvent->id) && hFunc(
                    currTime - contactor->I) > hFuncTh->get(currEvent->from,
                    currEvent->id))
                  {
                    infect(currEvent->id);
                    contactee->addContact(contactor, FEEDMILL, currEvent->time);
                  }
                else
                  {
                    contactee->addContact(contactor, FEEDMILL, currEvent->time);
                  }
              }

          }
        else if (contactee->isIAt(currTime))
          {
            if (contactor->isSAt(currTime)
                || contactor->isIAt(currTime))
              {
                contactee->addContact(contactor, FEEDMILL, currEvent->time);
              }
          }

        break;

      case SHOUSE:
        contactor = (*individuals)[currEvent->from];

        if (contactee->isSAt(currTime))
          {
            if (contactor->isSAt(currTime))
              {
                contactee->addContact(contactor, SHOUSE, currEvent->time);
              }
            else if (contactor->isIAt(currTime))
              {
                if (P2 > shThres->get(currEvent->from, currEvent->id) && hFunc(
                    currTime - contactor->I) > hFuncTh->get(currEvent->from,
                    currEvent->id))
                  {
                    infect(currEvent->id);
                    contactee->addContact(contactor, SHOUSE,currEvent->time);
                  }
                else
                  {
                    contactee->addContact(contactor, SHOUSE, currEvent->time);
                  }
              }
          }
        else if (contactee->isIAt(currTime))
          {
            if (contactor->isSAt(currTime)
                || contactor->isIAt(currTime))
              {
                contactee->addContact(contactor, SHOUSE, currEvent->time);
              }
          }

        break;

      case COMPANY:
        contactor = (*individuals)[currEvent->from];

        if (contactee->isSAt(currTime))
          {
            if (contactor->isIAt(currTime))
              {
                if (hFunc(currTime - contactor->I) > hFuncTh->get(
                    currEvent->from, currEvent->id))
                  infect(currEvent->id);
              }
          }

        break;

      case ISPATIAL:
        contactor = (*individuals)[currEvent->from];
        if (contactee->isSAt(currTime))
          {
            if (contactor->isIAt(currTime))
              {
                if (hFunc(currTime - contactor->I) > hFuncTh->get(
                    currEvent->from, currEvent->id))
                  infect(currEvent->id);
              }
          }

        break;

      case NSPATIAL:
        contactor = (*individuals)[currEvent->from];
        if (contactee->isSAt(currTime))
          {
            if (contactor->isNAt(currTime))
              infect(currEvent->id);
          }

        break;

        }

      currEvent = contactQueue->next();

    } // while(numN > 0 && currEvent != NULL)

  sort(individuals->begin(), individuals->end(), compInfectionTime);

}

void
SimOnContact::reset()
{
  // Resets the simulation to begin again

  currTime = 0.0;
  numS = nTotal;
  numI = 0;
  numN = 0;
  numR = 0;

  contactQueue->reset();
  notifications.clear();

  individuals->resetEventTimes();

}

void
SimOnContact::writeSimToFile(const string filename, const bool includeCensored, const bool includeDC) const
{
  // Write .ipt file

  ofstream output;
  output.open(filename.c_str(), ios::out);
  if (!output.is_open())
    {
      throw EpiRisk::output_exception(string("Cannot open " + filename + " for writing").c_str());
    }

  output.precision(9);
  for (Population::iterator iter = individuals->begin();
       iter != individuals->end();
       iter++)
    {
      if (iter->statusAt(maxTime) == Individual::SUSCEPTIBLE) continue;
      if (iter->N == maxTime && !includeCensored && !iter->isDC) continue;
      if (iter->isDC && !includeDC) continue;

      output << fixed << iter->label << " " << iter->I << " " << iter->N
            << " " << iter->R << endl;

    }

  output.close();

}

void
SimOnContact::writeCTToFile(const string filename) const
{
  ctWriter->writeToFile(filename.c_str());
}

bool
SimOnContact::compInfectionTime(const Individual& lhs, const Individual& rhs)
{
  return lhs.I < rhs.I;
}

bool
SimOnContact::CompNotificationTimePtr::operator ()(Individual* lhs,
    Individual* rhs) const
{
  return lhs->N < rhs->N;
}

void
SimOnContact::infect(const size_t label)
{
  // Infects an individual
  if (label >= individuals->size()) {
    stringstream msg;
    msg << "Individual " << label << " out of bound in pop of size " << individuals->size();
    throw EpiRisk::range_exception(msg.str().c_str());
  }
  if ((*individuals)[label]->statusAt(currTime) != Individual::SUSCEPTIBLE)
    throw logic_error("Individual to infect is not susceptible");

  (*individuals)[label]->I = currTime;

  DCData::iterator dcIter = dcData.find(label);

  if(dcIter == dcData.end()) {

    if (currTime + inTimes[label] < maxTime)
      {
        (*individuals)[label]->N = currTime + inTimes[label];
      }
    else
      (*individuals)[label]->N = maxTime;

    if ((*individuals)[label]->N + nrTime < maxTime)
      (*individuals)[label]->R = (*individuals)[label]->N + nrTime;
    else
      (*individuals)[label]->R = maxTime;

  }

  notifications.insert((*individuals)[label]);

  numS--;
  numI++;
  numN++;
}

void
SimOnContact::notify(const size_t label)
{

}

void
SimOnContact::remove(const size_t label)
{

}

void
SimOnContact::appendContact(size_t& contacteeId, size_t& contactorId,
    const char* type, const double& time, bool caused)
{
  // Adds contact tracing data
  contactData[contacteeId]->appendContact(contactorId, true, type, time, caused);
  contactData[contactorId]->appendContact(contacteeId, false, type, time, caused);
}

double
SimOnContact::hFunc(const double t)
{
  // Returns the hFunction
  double myExp = exp(nu * t);
  return myExp / (myExp + mu);
}
