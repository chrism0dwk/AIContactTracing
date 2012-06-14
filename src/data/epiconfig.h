/* ./src/data/epiconfig.h
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

#ifndef RESANREPICONFIG_H
#define RESANREPICONFIG_H

#include <map>
#include <string>

using namespace std;

namespace resanr {

/**
EpiConfig reads a RESANR MCMC (.rmc) config file.

	@author Chris Jewell <chris.jewell@warwick.ac.uk>
*/
class EpiConfig{
public:
    EpiConfig(const char* _filename);

    ~EpiConfig();
    
    const char* operator[] (const char* elementName);
    
  private:
    const char* const filename;
    
    map<string, string> configElements;
    
    void readConfigFile();
};

}

#endif
