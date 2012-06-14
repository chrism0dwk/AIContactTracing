/* ./src/data/config/epiMCMCConfig.h
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
/***************************************************************************
 *   Copyright (C) 2009 by Chris Jewell                                    *
 *   chris.jewell@warwick.ac.uk                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __EPIMCMCCONFIG_H_
#define __EPIMCMCCONFIG_H_

// STL
#include <string>
#include <map>
#include <set>
#include <list>

// Xerces-c
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>

#include <iostream>

// Local includes
#include "dataExceptions.h"

XERCES_CPP_NAMESPACE_USE;

using namespace std;



enum PRIORDIST
{
  // Currently allowed distributions
  GAMMA = 0,
  BETA
};


typedef list<string> ConfigErrorList;
typedef list<string> ConfigWarningList;


class ParmData;
class ConfigErrorHandler;
class ConfigPrior;

typedef map<string,double> ConfigTuningList;


class EpiMCMCConfig
{
  /** EpiMCMCConfig stores all the runtime configuration information 
  for epiMCMC **/
  
  public:
    
    // XML
    XercesDOMParser* parser;
    ConfigErrorHandler* errHandler;
    ConfigErrorList parseErrors;
    ConfigWarningList parseWarnings;
    DOMDocument* doc; // Pointer to the XML document
    
    //Constructor / Destructor
    EpiMCMCConfig();  // Creates an empty config
    EpiMCMCConfig(string& filename);  // Binds to a config XML file
    virtual ~EpiMCMCConfig();
    
    // XML handler methods
    virtual void load(string& filename);  // Loads config data, overwriting what's currently there
    virtual void save(string& filename);  // Saves the config to disk
    virtual const ConfigErrorList* getParseErrors();
    virtual const ConfigWarningList* getParseWarnings();

    // Getters
    int getNumberParameters(); // Gets the number of parameters
    set<string>* getParameterNames(); // Fetches a set of parameter names - raises a ConfigException if
                                      //  duplicate parameters are found.
    double getStartValue(string& parmName); // Gets the start value for parmName
    ConfigPrior getPrior(string& parmName); // Gets the prior for parmName
    ConfigTuningList getTuningList(string& parmName); // Gets the tuning constant list for parmName
    set<string> getOptionNames(); // Fetches a set of option names
    string getOptionValue(string& optionName); // Gets the value of an option
//    int getOptionValue(string& optionName);
//    double getOptionValue(string& optionName);
    set<string> getConstantNames(); // Fetches a set of option names
    double getConstantValue(); // Gets the value of a constant
    
};


/** ConfigErrorHandler handles errors from a DOMParser. **/

class ConfigErrorHandler : HandlerBase
{
  /** Implements error handler for EpiMCMCConfig XML parsing **/
  public:
    explicit ConfigErrorHandler(ConfigErrorList& _errors, ConfigWarningList& _warnings);
    virtual void error(const SAXParseException& exc);
    virtual void fatalError(const SAXParseException& exc);
    virtual void warning(const SAXParseException& exc);
    virtual void resetErrors();

  private:
    ConfigErrorList& errors;
    ConfigWarningList& warnings;
};



class ParmData
{
  /** Stores the parameter data:
  Start value
  Prior and hyperparameters
  Initial MH tuning constant
   **/
  public:
    ParmData(const PRIORDIST& _prior, const map<string,double> hp);
    virtual ~ParmData(){};
  
    void setMHTuning(const double& val);
    void setStartVal(const double& val);
    PRIORDIST getPriorDist();
    map<string,double>* getHyperParms();
    double getMHTuning();
    double getStartVal();
  
  private:
    PRIORDIST priordist;
    map<string,double> hyperParms;
    double mhTuning;
    double startVal;
};




#endif
