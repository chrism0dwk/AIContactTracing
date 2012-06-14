/* ./src/data/posterior.cpp
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

#include "posterior.h"

#include <string>
#include <string.h>
#include <cstdlib>
#include "stlStrTok.hpp"


#define LINEBUFFERSIZE 500

Posterior::Posterior()
{
}



Posterior::~Posterior()
{
}



int Posterior::initialize(const char* filename, const int skip)
{

  char linebuffer[LINEBUFFERSIZE];
  char* pch;

  cout << "Opening posterior file: " << filename << endl;

  mcmcFile.open(filename);
  if (!mcmcFile.is_open())
    {
      return(-1);
    }
  vector<double> parmLine;
  int counter=0;
  while (!mcmcFile.eof())
    {

      parmLine.clear();

      mcmcFile.getline(linebuffer, LINEBUFFERSIZE);
      if (strlen(linebuffer) == 0) break; // No characters so stop

      if (counter % skip == 0)
        {
          pch = strtok(linebuffer, " ");
          while (pch != NULL)
            {
              parmLine.push_back(atof(pch));
              pch = strtok(NULL," ");
            }
          posteriors.push_back(parmLine);
        }
      counter++;
    }
  cout << "READ IN " << counter << " LINES" << endl;
  mcmcFile.close();

  if (posteriors.size() > 0) return 0;
  else return 1;

}



int Posterior::fetch(vector<double>& parmRow, const int& i)
{
  // Fetches a row of parameters and returns them in a vector

  try
    {
      parmRow = posteriors.at(i);
      return 0;
    }
  catch (exception& e)
    {
      cout << "Exception in int Posterior::fetch(vector<double>& parmRow, const int& i): " << e.what() << endl;
      return -1;
    }

}



size_t Posterior::numIterations()
{
  /** Returns the number of MCMC iterations in
   * the posterior file */
  return posteriors.size();
}



