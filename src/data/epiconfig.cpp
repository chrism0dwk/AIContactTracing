/* ./src/data/epiconfig.cpp
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

#include "epiconfig.h"
#include "configExceptions.h"
#include "stlStrTok.hpp"

#include <fstream>
#include <string>

namespace resanr {

EpiConfig::EpiConfig(const char* _filename) : filename(_filename)
{
/** Initialise the class by opening and reading file _filename
 */
  readConfigFile();
}


EpiConfig::~EpiConfig()
{
}


const char* EpiConfig::operator[](const char* elementName)
{
  /** Returns the value of elementName in the config file */
  
  map<string, string>::iterator itElement;
  
  itElement = configElements.find(elementName);
  if(itElement == configElements.end()) {
    char msg[200];
    sprintf(msg,"Config Element '%s' not found",elementName);
    throw config_error(msg);
  }
  
  return itElement->second.c_str();
}


void EpiConfig::readConfigFile()
{
  // Reads the config file into a map.  Throws an exception if
  // duplicate entries are found in the config file.
  
  ifstream configFile;
  string linebuffer;
  vector<string> tokens;
  pair<map<string,string>::iterator,bool> rv;
  
//  configFile.exceptions(ifstream::failbit);// | ifstream::badbit);
  
  configFile.open(filename,ios::in);
  
  while(!configFile.eof())
  {
    getline(configFile,linebuffer);
    if(linebuffer.size() == 0) continue;
    if(linebuffer[0] == '#') continue;
    if(linebuffer[0] == ' ') continue;
    
    stlStrTok(tokens,linebuffer,"=");
    if(tokens.size() != 2) throw config_error("Malformed element in config file");
    
    rv = configElements.insert(pair<string,string>(tokens[0],tokens[1]));
    if (rv.second == false) throw config_error("Duplicate element in config file");
    
  }
  
  configFile.close();
  
}

}
