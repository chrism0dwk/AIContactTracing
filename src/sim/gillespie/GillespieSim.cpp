/* ./src/sim/gillespie/GillespieSim.cpp
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
// The code for AI_sim class.  See aisim.h for details

#include "GillespieSim.hpp"
#include "EpiRiskException.hpp"
#include "stlStrTok.hpp"

#define CT_PERIOD 21

// H-function = 1
//#define F_VALUE 0.25
//#define G_VALUE 0

// Varying H-function
#define F_VALUE 60.0
#define G_VALUE 1.3

/////////////////////////////////////////////////////////////////////
// Constructors / Destructors
/////////////////////////////////////////////////////////////////////

GillespieSim::GillespieSim(const size_t popSize, gsl_rng* rng) :
  contactWriter(0), rng(rng), init_done(0), NPARMS(16), rho(NULL), f(F_VALUE),
      g(G_VALUE), ctOutput(true), N_total(popSize)
{
  // Set default values
  setStartTime(0.0);
  setMaxTime(5000);

  // Set up Contact Writer
  contactWriter = new XmlCTWriter();

  // Call the reset function to
  // construct the population
  resetPopulation();
}

GillespieSim::~GillespieSim()
{
  gsl_rng_free(rng);
  delete[] rho;
}

void
GillespieSim::loadCovariates(const string dataPrefix)
{

  //! Loads in model covariates

  string filename;

  // Set up contact matrix
  filename = dataPrefix + ".fm";
  cerr << "Contact matrix: " << filename << endl;
  rv = fm_Mat.init(filename.c_str(), N_total);
  if (rv != 0)
    {
      throw EpiRisk::data_exception("Feed Mill Contact Matrix setup failed!");
    }

  filename = dataPrefix + ".sh";
  cerr << "Contact matrix: " << filename << endl;
  rv = sh_Mat.init(filename.c_str(), N_total);
  if (rv != 0)
    {
      throw EpiRisk::data_exception("SH Contact Matrix setup failed!");
    }

  filename = dataPrefix + ".cp";
  cerr << "Contact matrix: " << filename << endl;
  rv = cp_Mat.init(filename.c_str(), N_total);
  if (rv != 0)
    {
      throw EpiRisk::data_exception("Feed Mill Contact Matrix setup failed!");
    }

  // Set up frequency file

  //ifstream freqs;
  filename = dataPrefix + ".freq";
  cerr << "Frequency file: " << filename << endl;
  rv = freqInit(filename.c_str());
  if (rv != 0)
    {
      throw EpiRisk::data_exception("Freq table setup failed!");
    }

  // Set up Species matrix
  filename = dataPrefix + ".sp";
  cerr << "Species file: " << filename << endl;
  rv = species.initialize(filename.c_str(), N_total, 9);
  if (rv != 0)
    {
      throw EpiRisk::data_exception("Species table setup failed!");
    }

  // Set up spatial matrix
  filename = dataPrefix + "_dist.txt";
  cerr << "Distance file: " << filename << endl;
  rv = distanceInit(filename.c_str());

  init_done = 1;
  cout << "Finished model initialisation" << endl;

}

void
GillespieSim::loadEpiData(const string filename, const double obsTime, const double a, const double b, const double c)
{
  //! Loads existing epidemic data from a .ipt file

  // Open .ipt file
  ifstream iptFile;
  iptFile.open(filename.c_str(), ios::in);
  if (!iptFile.good())
    throw EpiRisk::data_exception("Cannot open epidemic data file");

  // Read in data
  string buffer;
  vector<string> tokens;
  size_t label;
  double I, N, R;
  while (!iptFile.eof())
    {
      getline(iptFile, buffer);
      if (buffer.size() < 2)
        break;
      stlStrTok(tokens, buffer, " ");
      if (tokens.size() != 4)
        throw EpiRisk::parse_exception("Malformed epidemic data");

      label = atoi(tokens[0].c_str());
      I = atof(tokens[1].c_str());
      N = atof(tokens[2].c_str());
      R = atof(tokens[3].c_str());

      if (I > obsTime)
        {
          stringstream msg;
          msg << "Individual " << label << " has infection time (" << I
              << ") > obsTime (" << obsTime << ")";
          throw logic_error(msg.str().c_str());
        }

      // Check what information we have actually observed
      if (N == obsTime)
        { // This means both N and R have not occurred yet.
          while (1)
            { // Rejection sampling.  Algorithm could well stick here!!
              N = I + rng_extreme(a, b);
              if (N > obsTime)
                break;
            }
          R = N + c;
        }
      else if (R == obsTime)
        {
          R = N + c;
        }

      individuals.at(label).I = I;
      individuals.at(label).N = N;
      individuals.at(label).R = R;
    }

}



void GillespieSim::loadCTData(const string filename)
{
  //! Loads in pre-existing contact tracing information

  contactWriter->readFromFile(filename,contactData);

}

void
GillespieSim::setIndexCase(const size_t indexCase, const double I, const double N, const double R)
{
  //! Sets the label of the index case
  I1 = indexCase;

  if (I1 >= individuals.size())
    {
      throw EpiRisk::range_exception("Selected I1 out of range");
    }
  individuals[indexCase].I = I;
  individuals[indexCase].N = N;
  individuals[indexCase].R = R;
  individuals[indexCase].status = individuals[indexCase].statusAt(startTime);
}



void
GillespieSim::setStartTime(const double startTime)
{
  //! Sets the time at the beginning of the simulation
  this->startTime = startTime;
}



double
GillespieSim::getStartTime()
{
  //! Returns the currently set start time
  return startTime;
}

void
GillespieSim::setMaxTime(const double maxTime)
{
  //! Sets the maximum time for which the simulation will run
  this->maxTime = maxTime;
}

double
GillespieSim::getMaxTime()
{
  //! Gets the currently set max time
  return maxTime;
}


void GillespieSim::setMaxN(const size_t maxNumberOfNotifications)
{
  //! Sets the max number of notificatino events
  maxN = maxNumberOfNotifications;
}

void
GillespieSim::loadDCData(const string filename)
{
  //! Loads Dangerous Contact cull data

  ifstream dcFile;
  dcFile.open(filename.c_str(), ios::in);
  if (!dcFile.good())
    {
      string msg = "Cannot open DC file: " + filename;
      throw EpiRisk::data_exception(msg.c_str());
    }

  string buffer;
  vector<string> tokens;
  size_t label;
  double N, R;

  while (!dcFile.eof())
    {
      getline(dcFile, buffer);
      if (buffer.size() < 2)
        continue; // Skip blank lines
      stlStrTok(tokens, buffer, " ");
      if (tokens.size() != 3)
        throw EpiRisk::parse_exception("Malformed data in DC file");

      label = atoi(tokens[0].c_str());
      N = atof(tokens[1].c_str());
      R = atof(tokens[2].c_str());

      //individuals.at(label).I = N;
      individuals.at(label).N = N;
      individuals.at(label).R = R;
      individuals.at(label).isDC = true;
    }
}

void
GillespieSim::addInfection(const size_t label, const double I)
{
  // Adds an infection (eg an I1)

  double I1_Ntime = rng_extreme(a, b);

  individuals.at(label).I = I;
  individuals.at(label).N = I + I1_Ntime;
  individuals.at(label).R = I + I1_Ntime + c;
  individuals.at(label).status = Individual::INFECTED;
  infective.insert(pair<double, Individual*> (individuals.at(label).N,
      &(individuals.at(label))));

  S--;
}

/////////////////////////////////////////////////////////////////////////
// Execution methods
/////////////////////////////////////////////////////////////////////////

void
GillespieSim::simulate(vector<double>& transmissionParms, const double my_a,
    const double my_b, const double my_c)
{
  // Runs an epidemic from scratch with a randomly assigned I1

  if (init_done != 1)
    {
      throw logic_error("Fatal error: AI_sim::run  called before AI_sim::init");
    }

  // Decls
  Population::iterator itIndiv;

  // Switch on ctOutput
  ctOutput = true;
  numFMInfecs = 0;
  numFMNonInfecs = 0;
  numSHInfecs = 0;
  numSHNonInfecs = 0;
  numOtherInfecs = 0;

  // Parameters
  beta = transmissionParms;
  a = my_a;
  b = my_b;
  c = my_c;

  for (size_t h = 0; h < NPARMS; ++h)
    {
      cout << "Beta" << h << ": " << beta[h] << endl;
    }
  cout << "a: " << a << endl;
  cout << "b: " << b << endl;
  cout << "c: " << c << endl;

  // Set up initial event queues
  infective.clear();
  notified.clear();
  S = N_total;
  R = 0;

  // Calculate initial state
  itIndiv = individuals.begin();
  pair<B_INDEX::iterator, bool> rv;
  while (itIndiv != individuals.end())
    {

      if (itIndiv->isDC)
        {
          // Add as a DC - for now DC's must have N=R
          S--;
          R++;
        }
      else
        {
          // Add as a normal infecton
          if (itIndiv->statusAt(startTime) == Individual::INFECTED)
            {
              rv = infective.insert(pair<double, Individual*> (itIndiv->N,
                  &(*itIndiv)));
              if (rv.second == false)
                {
                  stringstream msg;
                  msg << "Duplicate event time " << itIndiv->N
                      << " initialising infectives vector in "
                      << __PRETTY_FUNCTION__;
                  throw logic_error(msg.str().c_str());
                }
              S--;
            }
          else if (itIndiv->statusAt(startTime) == Individual::NOTIFIED) // Notified
            {
              rv = notified.insert(pair<double, Individual*> (itIndiv->R,
                  &(*itIndiv)));
              if (rv.second == false)
                {
                  stringstream msg;
                  msg << "Duplicate event time " << itIndiv->N
                      << " initialising infectives vector in "
                      << __PRETTY_FUNCTION__;
                  throw logic_error(msg.str().c_str());
                }
              S--;
            }
          else if (itIndiv->statusAt(startTime) == Individual::REMOVED) // Removed
            {
              S--;
              R++;
            }
        }
      itIndiv->status = itIndiv->statusAt(startTime);

      itIndiv++;
    }

  cout << "Simulating from model........." << endl;


  // Set curr_time
  curr_time = startTime;

  // Set up contact index
  contactCDFInit();
  contactCDF = contactCDFCached;

  execute(); // Execute the simulation

}

void
GillespieSim::resetPopulation()
{
  //! Clears the current epidemic population

  resetEpidemic();


  // Set up the vanilla population
  individuals.clear();
  for (size_t i = 0; i < N_total; ++i)
    {
      individuals.push_back(Individual(i, GSL_POSINF));
    }

  // Contact tracing
  resetCTData();
}

void
GillespieSim::resetEpidemic()
{
  //! Resets the epidemic data only
  S = 0;
  infective.clear();
  notified.clear();
  R = 0;
}


void
GillespieSim::resetCTData()
{
  // (Re)initialize CT data
  Population::iterator itIndiv = individuals.begin();
  contactData.clear();
  while (itIndiv != individuals.end())
    {
      contactData.insert(pair<int, XmlCTData*> (itIndiv->label,
          contactWriter->createCTData(itIndiv->label)));
      itIndiv++;
    }
}


void
GillespieSim::writeSimToFile(const string filename, const bool includeCensored,
    const bool includeDC) const
{

  ofstream output;

  output.open(filename.c_str());
  if (!output.is_open())
    {
      string msg = "Cannot open " + filename + " for writing.";
      throw EpiRisk::output_exception(msg.c_str());
    }

  output.precision(9);
  for (Population::const_iterator iter = individuals.begin(); iter
      != individuals.end(); iter++)
    {
      if (iter->statusAt(curr_time) == Individual::SUSCEPTIBLE)
        continue;
      if (iter->statusAt(curr_time) == Individual::INFECTED && !includeCensored && !iter->isDC)
        continue;
      if (iter->isDC && !includeDC)
        continue;

      output << fixed
             << iter->label
             << " " << iter->I
             << " " << (iter->N < maxTime ? iter->N : maxTime)
             << " " << (iter->R < maxTime ? iter->R : maxTime)
             << endl;

    }

  output.close();

}

void
GillespieSim::writeCTToFile(const string filename, const bool censored)
{
  //! Writes the contact tracing data to filename

  // First empty the DOM
  contactWriter->resetDOM();

  Population::iterator itPopn = individuals.begin();
  while(itPopn != individuals.end()) {
    if (itPopn->statusAt(curr_time) == Individual::NOTIFIED ||
        itPopn->statusAt(curr_time) == Individual::REMOVED) {
      publishContacts(&(*itPopn),itPopn->N - CT_PERIOD);
    }
    else {
      if (censored) publishContacts(&(*itPopn),(maxTime < curr_time ? maxTime : curr_time));
    }

    itPopn++;
  }

  try
    {
      contactWriter->writeToFile(filename.c_str());
    }
  catch (const exception& e)
    {
      string msg = "Error writing contact tracing information: " + string(
          e.what());
      throw EpiRisk::output_exception(msg.c_str());
    }

  ofstream output;
  output.open(filename.c_str(), ios::app);
  output << "\n<!-- Num FM infecs: " << numFMInfecs << "-->\n";
  output << "\n<!-- Num FM Non-infecs: " << numFMNonInfecs << "-->\n";
  output << "<!-- Num SH infecs: " << numSHInfecs << "-->\n";
  output << "<!-- Num SH Non-infecs: " << numSHNonInfecs << "-->\n";
  output << "<!-- Num Other infecs: " << numOtherInfecs << "-->" << endl;
  output.close();
}

size_t
GillespieSim::totalSize()
{
  // Returns the total size of the epidemic

  return R;
}


bool
GillespieSim::isEpidemicOver()
{
  // Returns true if the epidemic is over

  if (infective.empty() && notified.empty()) return true;
  else return false;
}

///////////////////////////////////////////////////////////////////////////////////
// Maths functions
///////////////////////////////////////////////////////////////////////////////////

inline double
GillespieSim::fmRate(const size_t& i, const size_t& j)
{
  // Rate at which contacts occur via feed mills

  return fm_Mat.isConn(i, j) * 10 * (0.5 * cFreq.at(j).fm * (3
      / (cFreq.at(j).fm_N)));
}

inline double
GillespieSim::fmInfecRate(const size_t& i, const size_t& j)
{
  // Rate at which infections occur via feed mills

  return beta[1] * fmRate(i, j);
}

inline double
GillespieSim::shRate(const size_t& i, const size_t& j)
{
  // Rate at which slaughterhouse contacts occur

  return sh_Mat.isConn(i, j) * 10 * (0.5 * cFreq[j].sh * (3 / (cFreq[j].sh_N)));
}

inline double
GillespieSim::shInfecRate(const size_t& i, const size_t& j)
{
  // Slaughterhouse contact freq between i and j

  return beta[2] * shRate(i, j);
}

inline double
GillespieSim::cpInfecRate(const size_t& i, const size_t& j)
{
  // Company contact freq - currently either 0 or 1

  return beta[3] * cp_Mat.isConn(i, j);
}

inline double
GillespieSim::spatialRateI(const size_t& i, const size_t& j)
{
  // Spatial infection rate if i infected

  return beta[4] * exp(-beta[6] * (*(rho + i + N_total * j) - 5));
}

inline double
GillespieSim::spatialRateN(const size_t& i, const size_t& j)
{
  // Spatial infection rate if i infected

  return beta[5] * exp(-beta[6] * (*(rho + i + N_total * j) - 5));
}

inline double
GillespieSim::betaij(const size_t i, const size_t j)
{
  double betaij = 0.0;

  if (i > N_total - 1 || j > N_total - 1)
    {
      char errMsg[200];
      sprintf(errMsg, "Range overflow in betaij.  i = %lu, j = %lu", i, j);
      throw range_error(errMsg);
    }

  // Feed Mills
  betaij += fmRate(i, j);

  // Slaughterhouse
  betaij += shRate(i, j);

  // Company
  betaij += cpInfecRate(i, j);

  // Spatial
  betaij += spatialRateI(i, j);

  // Species susceptibility
  for (size_t k = 7; k < NPARMS; ++k)
    {
      if (species.at(j, k - 7) == 1)
        {
          betaij *= beta[k];
        }
    }

  return betaij;
}

inline double
GillespieSim::betaijstar(const size_t i, const size_t j)
{
  double betaijstar = 0.0;
  if (i > N_total - 1 || j > N_total - 1)
    throw range_error("betaijstar");

  // Spatial
  betaijstar += spatialRateN(i, j);

  // Species
  for (size_t k = 7; k < NPARMS; ++k)
    {
      if (species.at(j, k - 7) == 1)
        {
          betaijstar *= beta[k];
        }
    }

  return betaijstar;
}

inline double
GillespieSim::speciesj(const size_t& j)
{
  for (size_t k = 7; k < NPARMS; ++k)
    {
      if (species.at(j, k - 7) == 1)
        {
          return beta[k];
        }
    }
  return 1.0;
}

double
GillespieSim::rng_extreme(double a, double b)
{
  return 1 / b * log(1 - log(1 - gsl_rng_uniform(rng)) / a);
}

size_t
GillespieSim::rng_ztPois(double lambda)
{
  // Generates a realisation from a zero-truncated
  // Poisson random variable with mean lambda
  // using inverse CDF method

  double normConst = 1 - exp(-lambda);
  size_t j = 1;
  double p = lambda * exp(-lambda) / normConst;
  double F = p;

  double u = gsl_rng_uniform(rng);

  while (u > F)
    {
      p *= lambda / (j + 1);
      F += p;
      j++;
    }

  return j;
}

double
GillespieSim::hFunc(const double t)
{
  if (t < 0)
    {
      throw logic_error("t < 0 in hFunc!");
    }
  double expg = exp(g * t);
  return expg / (f + expg);
}

/////////////////////////////////////////////////////////////////////
// Initialisation methods
/////////////////////////////////////////////////////////////////////

int
GillespieSim::freqInit(const string filename)
{

  ifstream datafile;
  char line[200];
  char element[10];
  char *line_ptr;
  char *element_ptr;
  frequencies freqRow;

  cFreq.clear();

  datafile.open(filename.c_str(), ios::in);
  if (!datafile.is_open())
    {
      throw EpiRisk::data_exception("Cannot open frequency file");
    }

  while (1)
    {

      datafile.getline(line, 200);
      if (datafile.eof())
        break;
      line_ptr = line;
      element_ptr = strchr(line_ptr, ' ');
      *element_ptr = '\0';
      strcpy(element, line_ptr);
      freqRow.fm = (freq_t) atof(element);
      line_ptr = element_ptr + 1;
      element_ptr = strchr(line_ptr, ' ');
      *element_ptr = '\0';
      strcpy(element, line_ptr);
      freqRow.fm_N = atof(element);
      line_ptr = element_ptr + 1;
      element_ptr = strchr(line_ptr, ' ');
      *element_ptr = '\0';
      strcpy(element, line_ptr);
      freqRow.sh = (freq_t) atof(element);
      line_ptr = element_ptr + 1;
      strcpy(element, line_ptr);
      freqRow.sh_N = atof(element);
      cFreq.push_back(freqRow);
      //cout << freqRow.fm << " " << freqRow.fm_N << " " << freqRow.sh << " " << freqRow.sh_N << endl;
    }

  datafile.close();

  return (0);
}

int
GillespieSim::distanceInit(const string filename)
{

  ifstream datafile;
  int i, j;
  double dist;
  char line[200];
  char element[10];
  char *line_ptr;
  char *element_ptr;

  // Allocate distance matrix
  if (rho != NULL)
    delete[] rho;
  else
    rho = new float[N_total * N_total];

  // Set all elements of rho to GSL_POS_INF
  for (size_t h = 0; h < N_total * N_total; ++h)
    {
      *(rho + h) = GSL_POSINF;
    }

  datafile.open(filename.c_str(), ios::in);
  if (!datafile.is_open())
    {
      throw EpiRisk::data_exception("Cannot open distance file");
    }

  while (1)
    {

      datafile.getline(line, 200);
      if (datafile.eof())
        break;
      line[strlen(line) - 1] = '\0';
      line_ptr = line;
      element_ptr = strchr(line_ptr, ' ');
      *element_ptr = '\0';
      strcpy(element, line_ptr);
      i = atoi(element);
      line_ptr = element_ptr + 1;
      element_ptr = strchr(line_ptr, ' ');
      *element_ptr = '\0';
      strcpy(element, line_ptr);
      j = atoi(element);
      line_ptr = element_ptr + 1;
      strcpy(element, line_ptr);
      dist = atof(element);
      *(rho + i + N_total * j) = dist;

    }

  datafile.close();

  return (0);
}

void
GillespieSim::contactCDFInit()
{
  // Initialises the cached contactCDF

  sum_beta = 0.0;
  vector<Individual>::iterator itReceiver = individuals.begin();
  vector<Individual>::iterator itSender;
  while (itReceiver != individuals.end())
    {
      if (itReceiver->statusAt(startTime) == Individual::NOTIFIED
          || itReceiver->statusAt(startTime) == Individual::REMOVED) {
        itReceiver++;
        continue;
      }

      sum_beta += beta[0];
      itSender = individuals.begin();
      while (itSender != individuals.end())
        {
          if (itReceiver != itSender)
            {
              if (itSender->statusAt(startTime) == Individual::SUSCEPTIBLE ||
                  itSender->statusAt(startTime) == Individual::INFECTED)
                sum_beta += betaij(itSender->label, itReceiver->label);
              else if (itSender->statusAt(startTime) == Individual::NOTIFIED)
                sum_beta += betaijstar(itSender->label, itReceiver->label);
            }
          itSender++;
        }
      contactCDFCached.insert(pair<double, Individual*> (sum_beta,
          &(*itReceiver)));
      itReceiver++;
    }

}

/////////////////////////
// Getters and setters //
/////////////////////////


void
GillespieSim::execute()
{
  // Now we begin our simulation loop


  CONTACT contact;
  double eventTime;
  EVENTTYPE eventType;
  Individual* pEventIndiv;

  cout << "Here we go...." << endl;
  while (1)
    {

      if (curr_time > maxTime)
        break; // End of simulation

      if (notified.size() + R > maxN)
        break;

      if (infective.empty() && notified.empty())
        {
          cout << "No infectives or notifieds left.  Epidemic over :-) "
              << endl;
          break;
        }

      eventTime = GSL_POSINF;

      // First we calculate time to next contact
      if (S + infective.size() + notified.size() != 0)
        { // If we have possible contacts
          eventTime = getTimeToNextContact() + curr_time;
          contact.receiver = getReceiver();
          contact.sender = getSender(contact.receiver);
          contact.method = getContactMethod(contact.sender, contact.receiver);
          contact.time = eventTime;

          // Work out if this contact was infectious
          if (isInfectious(contact))
            eventType = INFECTIONEVENT;
          else
            eventType = CONTACTEVENT;

          pEventIndiv = contact.receiver;
        }

      // Evaluate Time to next Notification
      B_INDEX::iterator itInfective = infective.begin();
      if (itInfective->first < eventTime && !infective.empty())
        {
          eventTime = itInfective->first;
          eventType = NOTIFICATIONEVENT;
          pEventIndiv = itInfective->second;
        }

      // Evaluate Time to next Removal
      B_INDEX::iterator itNotified = notified.begin();
      if (itNotified->first < eventTime && !notified.empty())
        {
          eventTime = itNotified->first;
          eventType = REMOVALEVENT;
          pEventIndiv = itNotified->second;
        }

      // We see which event has occurred first, and update the corresponding populations
      assert(eventTime > curr_time);
      curr_time = eventTime; // Update current time

      switch (eventType)
        {

      case CONTACTEVENT:
        assert(contact.receiver->status != Individual::NOTIFIED); // No received contacts
        assert(contact.receiver->status != Individual::REMOVED); // if after notification

        if (contact.sender != NULL && contact.sender->status
            == Individual::INFECTED)
          addContact(contact, false);
        break;

      case INFECTIONEVENT:
        infect(pEventIndiv);
        addContact(contact, true);
        break;

      case NOTIFICATIONEVENT:
        notify(pEventIndiv);
        //publishContacts(pEventIndiv,pEventIndiv->N - CT_PERIOD);
        break;

      case REMOVALEVENT:
        remove(pEventIndiv);
        break;

      default:
        //BP
        throw logic_error("No such event!");

        }

      // Update sum_beta
      sum_beta = beta_max();

      addResult(eventType, pEventIndiv);

      //cout << "\r" << curr_time;

      // A trap for the unwary - we should *never* have to use this!
      if (S + infective.size() + notified.size() + R != N_total)
        {
          char errMsg[500];
          sprintf(
              errMsg,
              "There is something definitely wrong!\n\tS=%i, I=%lu, N=%lu, R=%i\n",
              S, infective.size(), notified.size(), R);
          cerr << errMsg << endl;
          throw logic_error(errMsg);
        }

      if (S + infective.size() != contactCDF.size())
        {
          cout << "S+infective.size() != contactCDF.size() in "
              << __PRETTY_FUNCTION__ << endl;
        }

    } // while(1)

  cout << endl;

  //cout << "Finished running model" << endl;
  cout << R << " individuals removed out of " << N_total << endl;

}

/////////////////////////////////////////////////////////////////////////////////
// Stochastic methods ///////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

double
GillespieSim::getTimeToNextContact()
{
  // Simulates time to next infection ~Exp( 1/\sum_{j} (\beta_0 + \sum_{i} \beta_ij) )

  return gsl_ran_exponential(rng, 1 / beta_max());
}

Individual*
GillespieSim::getReceiver()
{
  // Returns a pointer to the next person contacted in the individuals vector

  double u = gsl_ran_flat(rng, 0, beta_max());

  return contactCDF.lower_bound(u)->second;
}

Individual*
GillespieSim::getSender(const Individual* const pReceiver)
{
  // Simulates a Sender given a Receiver.  Uses
  // inverse CDF sampling.  Runs in linear time
  // wrt S+I+N.

  map<double, Individual*> infector;
  map<double, Individual*>::iterator choose;
  double sum_i = 0.0;
  vector<Individual>::iterator itSender;

  // Construct CDF

  // Do \beta_0 infection
  sum_i += beta[0];
  infector.insert(pair<double, Individual*> (sum_i, NULL));

  // Now the rest of the CDF
  itSender = individuals.begin();
  while (itSender != individuals.end())
    {

      // Conditions
      if (itSender->label != pReceiver->label && // sender != receiver
          itSender->status != Individual::REMOVED)
        { // "I see dead people" ;-)

          if (itSender->status == Individual::NOTIFIED)
            {
              sum_i += betaijstar(itSender->label, pReceiver->label); //Add on beta_{ij}
            }
          else
            {
              sum_i += betaij(itSender->label, pReceiver->label);
            }
          infector.insert(pair<double, Individual*> (sum_i, &(*itSender)));
        }
      itSender++;
    }

  // Sample using inverse CDF method
  if (infector.empty())
    cout << "WARNING: empty infector map in " << __PRETTY_FUNCTION__ << endl;

  double u = gsl_ran_flat(rng, 0, sum_i);

  return infector.lower_bound(u)->second;
}

CONTYPE
GillespieSim::getContactMethod(const Individual* const pSender,
    const Individual* const pReceiver)
{
  // Simulates a contact type between sender and receiver
  // Uses inverse CDF

  map<double, CONTYPE> cdf;
  double cumSum = 0.0;
  double u;

  // Null sender implies \beta_0 infection
  if (pSender == NULL)
    return OTHER;

  // Susceptibles and infectives can contact by all methods
  if (pSender->status == Individual::INFECTED || pSender->status
      == Individual::SUSCEPTIBLE)
    {

      // Construct CDF
      cumSum += fmRate(pSender->label, pReceiver->label); // FM
      cdf.insert(pair<double, CONTYPE> (cumSum, FEEDMILL));

      cumSum += shRate(pSender->label, pReceiver->label); // SH
      cdf.insert(pair<double, CONTYPE> (cumSum, SHOUSE));

      cumSum += cpInfecRate(pSender->label, pReceiver->label); // OTHER
      cumSum += spatialRateI(pSender->label, pReceiver->label);
      cdf.insert(pair<double, CONTYPE> (cumSum, OTHER));

      // Return event by inverse CDF sampling
      u = gsl_ran_flat(rng, 0, cumSum);
      return cdf.lower_bound(u)->second;
    }

  // If sender is notified, can only contact by OTHER
  else if (pSender->status == Individual::NOTIFIED)
    {
      return OTHER;
    }

  // Trap for invalid disease stati (ie REMOVED)
  else
    {
      char msg[200];
      sprintf(msg, "Infection source invalid state in %s", __func__);
      throw logic_error(msg);
    }

}

bool
GillespieSim::isInfectious(const CONTACT& contact)
{
  // Decides whether our contact is infectious or not

  // Checks
  assert(contact.receiver->status != Individual::NOTIFIED);
  assert(contact.receiver->status != Individual::REMOVED);

  //if(contact.sender == NULL) cout << "WARNING: contact.sender is NULL in " << __PRETTY_FUNCTION__ << endl;
  // Algorithm:

  // Cannot be infected if receiver not susceptible
  if (contact.receiver->status != Individual::SUSCEPTIBLE)
    {
      return false;
    }

  if (contact.sender != NULL)
    { // NOT \beta_0 infection.

      // Check:
      assert(contact.sender->status != Individual::REMOVED);

      if (contact.sender->status == Individual::SUSCEPTIBLE)
        {
          return false; // Can't be infected from a susceptible
        }
      else if (contact.sender->status == Individual::INFECTED)
        {
          double infectiousness;
          infectiousness = hFunc(contact.time - contact.sender->I);

          // Modify for FEEDMILL and SHOUSE
          if (contact.method == FEEDMILL)
            infectiousness *= beta[1];
          else if (contact.method == SHOUSE)
            infectiousness *= beta[2];

          // Choose whether we have an infection:
          if (gsl_ran_flat(rng, 0, 1) < infectiousness)
            {
              if (hFuncOut.is_open())
                {
                  hFuncOut << contact.receiver->label << " "
                      << contact.sender->label << " " << contact.method << " "
                      << hFunc(contact.time - contact.sender->I) << " 1"
                      << endl;
                }
              if (contact.method == FEEDMILL)
                numFMInfecs++;
              else if (contact.method == SHOUSE)
                numSHInfecs++;
              return true;
            }
          else
            {
              if (hFuncOut.is_open())
                {
                  hFuncOut << contact.receiver->label << " "
                      << contact.sender->label << " " << contact.method << " "
                      << hFunc(contact.time - contact.sender->I) << " 0"
                      << endl;
                }
              if (contact.method == FEEDMILL)
                numFMNonInfecs++;
              else if (contact.method == SHOUSE)
                numSHNonInfecs++;
              return false;
            }
        }
      else
        { // NOTIFIED
          return true;
        }
    }
  else
    { // \beta_0 infection
      return true;
    }
  cout << "Outside if statement in " << __PRETTY_FUNCTION__ << endl;
  return false;

}

/////////////////////////////////////////////////////////////////////////////////
// Population maintenance methods
/////////////////////////////////////////////////////////////////////////////////

void
GillespieSim::infect(Individual* pInfection)
{

  // Simulate new notification time and add to the infected index:
  double notifyTime = rng_extreme(a, b) + curr_time;
  pair<B_INDEX::iterator, bool> rv = infective.insert(
      pair<double, Individual*> (notifyTime, pInfection));

  // Ensure the addition
  if (!rv.second)
    {
      char msg[200];
      sprintf(msg,
          "%lu - DUPLICATE KEY! Tried to insert %f and %lu has the same time!",
          pInfection->label, notifyTime, infective[notifyTime]->label);
      throw logic_error(msg);
    }

  // Update the status of our individual
  pInfection->status = Individual::INFECTED;
  pInfection->I = curr_time;
  pInfection->N = notifyTime;

  // Update S
  --S;

  return;

}

void
GillespieSim::notify(Individual* const pInfection)
{

  // 1) We *know* that the notification is the first in the infection index
  //           - ie the earliest notification time
  //           - needs to be removed from this index
  const B_INDEX::iterator itInfectiveLocn(infective.begin());

  // Trap to ensure consistency
  if (pInfection->label != itInfectiveLocn->second->label)
    {
      throw logic_error("Notification pointers do not match in function notify");
    }

  // Test to make sure we're notifying an infected
  if (pInfection->status != Individual::INFECTED)
    throw logic_error("State transition error: Can only notify an infection");

  // 2) We require to know the whereabouts of this individual in the contactCDF
  //           - we need to remove it from this index.
  B_INDEX::iterator itInfection = contactCDF.begin();
  while (itInfection != contactCDF.end())
    {
      if (itInfection->second->label == pInfection->label)
        break;
      else
        itInfection++;
    }

  // Trap in case we've not found our required individual
  if (itInfection == contactCDF.end())
    {
      throw logic_error("Label not found in function infect");
    }

  // Update the contactCDF index:
  double myPressure, modPressure;
  B_INDEX::iterator itLocal = itInfection; // Local iterator to modify

  //    1) get the pressure on our required individual: (p(indiv) - p(indiv--)
  if (itLocal == contactCDF.begin())
    myPressure = itLocal->first;
  else
    {
      myPressure = itLocal->first;
      itLocal--; // Go up one
      myPressure -= itLocal->first; // Get pressure just on our required.
      itLocal++; // Reset iterator
    }

  //    2) Modify the contact rates for notifieds

  itLocal = contactCDF.begin();
  modPressure = 0.0;
  while (itLocal != contactCDF.end())
    {
      if (itLocal != itInfection)
        { // Skip the notification
          modPressure -= betaij(pInfection->label, itLocal->second->label);
          modPressure += betaijstar(pInfection->label, itLocal->second->label);
          const_cast<double&> (itLocal->first) += modPressure;
        }
      itLocal++;
    }

  //    3) Remove pressure on itInfection from CDF
  itLocal = itInfection;
  itLocal++;
  while (itLocal != contactCDF.end())
    {
      const_cast<double&> (itLocal->first) -= myPressure;
      itLocal++;
    }

  // 4) Erase the notification from the CDF and infective
  contactCDF.erase(itInfection);
  infective.erase(itInfectiveLocn);

  // Update the individual's status:
  double removeTime = c + pInfection->N;
  pInfection->R = removeTime;
  pInfection->status = Individual::NOTIFIED;
  pair<B_INDEX::iterator, bool> rv = notified.insert(
      pair<double, Individual*> (removeTime, pInfection));

  // Test for a successful insertion
  if (!rv.second)
    throw logic_error(
        "Duplicate key during insertion to notified index in function notify");

  // Update sum_beta

  itLocal = contactCDF.end();
  itLocal--;
  sum_beta = itLocal->first;

  return;
}

void
GillespieSim::remove(Individual* pInfection)
{

  // Trap to prevent us being called if there is nobody to notify
  if (notified.size() < 1)
    throw logic_error("Notifieds < 1 and remove was called");

  // Trap to ensure consistency
  if (pInfection != notified.begin()->second)
    throw logic_error(
        "Argument to function remove is not equal to next removal");

  // Subtract the pressure from the notified in contactCDF
  B_INDEX::iterator itCDF = contactCDF.begin();
  double pressure = 0.0;
  while (itCDF != contactCDF.end())
    {
      pressure += betaijstar(pInfection->label, itCDF->second->label);
      const_cast<double&> (itCDF->first) -= pressure;
      itCDF++;
    }

  // Update individuals vector
  pInfection->status = Individual::REMOVED;

  // Now update the epidemic
  notified.erase(notified.begin()); // Erase the first element
  ++R;

  return;
}

/////////////////////////////////////////////////////////////////////////////////////
// Simulation output methods
/////////////////////////////////////////////////////////////////////////////////////

void
GillespieSim::addResult(EVENTTYPE event, Individual* pIndiv)
{

  struct result_row newRow;

  if (event == INFECTIONEVENT)
    newRow.event = 'i';
  else if (event == NOTIFICATIONEVENT)
    newRow.event = 'n';
  else if (event == REMOVALEVENT)
    newRow.event = 'r';
  else
    return;

  newRow.event_time = curr_time;
  newRow.label = pIndiv->label;
  newRow.S = S;
  newRow.I = infective.size();
  newRow.N = notified.size();
  newRow.R = R;

  result.push_back(newRow);

}

////////////////////////////////////////////////////////////////////////////////////
// Contact tracing methods
////////////////////////////////////////////////////////////////////////////////////


void
GillespieSim::addContact(CONTACT& contact, bool caused)
{
  // Adds a contact to the contacts data
  //   only logs FEEDMILL or SHOUSE contacts.

  // Don't add anything if we've a \beta_0 infection
  //   as no contact was made.
  if (contact.sender == NULL)
    return;

  XmlCTData* myCTData = NULL;

  // Do the incoming contact
  ContactData::const_iterator found = contactData.find(contact.receiver->label);
  if( found == contactData.end()) {
    stringstream msg;
    msg << "Cannot find receiver label: " << contact.receiver->label << " in contactData";
    cerr << msg.str() << endl;
    throw logic_error(msg.str().c_str()); // Make sure we've found the individual in the contact list
  }

  myCTData = found->second;
  if (contact.method == FEEDMILL || contact.method == SHOUSE)
    {
      myCTData->appendContact(contact.sender->label, true, conTypeToText(
          contact.method), curr_time, caused);

    }

  // Do the outgoing contact
  myCTData = contactData[contact.sender->label];
  if (contact.method == FEEDMILL || contact.method == SHOUSE)
    {
      myCTData->appendContact(contact.receiver->label, false, conTypeToText(
          contact.method), curr_time, caused);
    }

}

void
GillespieSim::publishContacts(Individual* pIndiv, const double startTime)
{

  // Creates contact tracing data for the current
  // notification and outputs it to the DOM tree

  // Check we're a notification event
//  if (pIndiv->status != Individual::NOTIFIED)
//    {
//      char msg[200];
//      sprintf(msg, "%s must only be used for a NOTIFICATIONEVENT event!",
//          __func__);
//      throw logic_error(msg);
//    }

  XmlCTData* myCTData = NULL;
  myCTData = contactData[pIndiv->label];

  //if(!myCTData->hasContacts()) return; //...before we waste any more compute cycles


  // Set the contactStart, and truncate contacts before this time.
  //double myContactStart = pIndiv->N - CT_PERIOD;
  myCTData->setCTStartTime(startTime);
  myCTData->truncate();

  // Add ctData to the contact tracing if not empty
  contactWriter->addCTData(myCTData);
}

const char*
GillespieSim::conTypeToText(CONTYPE& t)
{
  // Returns a string corresponding to connection type

  switch (t)
    {

  case FEEDMILL:
    return "FEEDMILL";

  case SHOUSE:
    return "SHOUSE";

  case OTHER:
    return "OTHER";

  default:
    throw logic_error("Invalid Contype");

    }

}

///////////////////////////////////////////////////////////////////////////////////////////
// Diagnostics methods
///////////////////////////////////////////////////////////////////////////////////////////

void
GillespieSim::getIndividual(size_t label)
{
  vector<Individual>::iterator it = individuals.begin();

  while (it != individuals.end())
    {
      if (it->label == label)
        break;

      cout << "Label: " << it->label << " I = " << it->I << " N = " << it->N
          << " R = " << it->R << endl;
      it++;
    }
}

void
GillespieSim::dumpInfective()
{
  //! Dumps infectives to stdout

  B_INDEX::iterator iIter = infective.begin();

  while (iIter != infective.end())
    {
      cout << "Label: " << iIter->second->label << ", time: " << iIter->first
          << endl;
      iIter++;
    }
}

void
GillespieSim::dumpNotified()
{
  //! Dumps infectives to stdout

  B_INDEX::iterator iIter = notified.begin();

  while (iIter != notified.end())
    {
      cout << "Label: " << iIter->second->label << ", beta: " << iIter->first
          << endl;
      iIter++;
    }
}

void
GillespieSim::dumpCDF()
{
  //! Dumps the contact CDF to stdout

  B_INDEX::iterator itContactCDF = contactCDF.begin();

  while (itContactCDF != contactCDF.end())
    {
      cout << itContactCDF->second->label << ": " << itContactCDF->first
          << endl;
      itContactCDF++;
    }
}

////////////////////////////////////////////////////////////////////////////////////
// Class Epidemic
////////////////////////////////////////////////////////////////////////////////////

double
GillespieSim::beta_max()
{

  B_INDEX::iterator iter = contactCDF.end();
  if (!contactCDF.empty())
    {
      iter--;
      return (iter->first);
    }
  else
    return 0.0;
}

//////////////////////////////////////////////////////////////////////////
// Class individual
//////////////////////////////////////////////////////////////////////////

Individual::Individual(size_t _label, double t) :
  label(_label), I(t), N(t), R(t), isDC(false), status(SUSCEPTIBLE)
{
}

bool
Individual::isSAt(const double time) const
{
  return I > time;
}

bool
Individual::isIAt(const double time) const
{
  return I <= time && time < N;
}

bool
Individual::isNAt(const double time) const
{
  return N <= time && time < R;
}

bool
Individual::isRAt(const double time) const
{
  return R <= time;
}

Individual::INFECSTATUS
Individual::statusAt(const double time) const
{
  //! Returns the status of an individual at time

  if (time < I)
    return SUSCEPTIBLE;
  else if (I <= time && time < N)
    return INFECTED;
  else if (N <= time && time < R)
    return NOTIFIED;
  else
    return REMOVED;
}
