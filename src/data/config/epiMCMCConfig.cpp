/* ./src/data/config/epiMCMCConfig.cpp
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

#include <string>
#include <sstream>
#include <iostream>
#include <memory>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include "epiMCMCConfig.h"

using namespace std;

EpiMCMCConfig::EpiMCMCConfig()
{
  /** Does nothing for the moment. Will most likely load in a template for
  an empty config document **/
}

EpiMCMCConfig::EpiMCMCConfig(string& filename) :
    parser(NULL)
{
  /** Binds the class to a config file on disc **/
  
  try {
    XMLPlatformUtils::Initialize();
  }
  catch (const XMLException& toCatch) {
    throw ConfigParserException(XMLString::transcode(toCatch.getMessage()));
  }
  
  load(filename);
}



void EpiMCMCConfig::load(string& filename) {
  
  /** Loads an epiRisk config file from disc **/
  
  if(parser != NULL) {
    delete parser;
    parser = NULL;
  }
  
  parser = new XercesDOMParser();
  parser->setValidationScheme(XercesDOMParser::Val_Always);
  parser->setValidationSchemaFullChecking(true);
  

  errHandler = (ConfigErrorHandler*) new ConfigErrorHandler(parseErrors,parseWarnings);
  parser->setErrorHandler((ErrorHandler*)errHandler);

  try {
    
    // Parse the document
    std::cout << "Parsing: " << filename.c_str() << std::endl;
    parser->parse(filename.c_str());
    
    // Get the document
    doc = parser->getDocument();
  }
  catch (const XMLException& toCatch) {
    throw ConfigXmlException(XMLString::transcode(toCatch.getMessage()));
  }
  catch (const DOMException& toCatch) {
    throw ConfigParserException(XMLString::transcode(toCatch.getMessage()));
  }
  catch (const SAXParseException& toCatch) {
    char* err = XMLString::transcode(toCatch.getMessage());
    stringstream ss;
    ss << err << " at line " << toCatch.getLineNumber() << ", column " << toCatch.getColumnNumber();
    XMLString::release(&err);
    
    throw ConfigParserException(ss.str().c_str());
  }
  catch (...) {
    throw ConfigUnknownException("Error loading config data.  Check the filename - is it correct?");
  }
  

}



EpiMCMCConfig::~EpiMCMCConfig()
{
  
  if(parser != NULL) {
    delete parser;
    parser = NULL;
  }
  
  XMLPlatformUtils::Terminate();
}



const ConfigErrorList* EpiMCMCConfig::getParseErrors()
{
  return &parseErrors;
}



const ConfigWarningList* EpiMCMCConfig::getParseWarnings()
{
  return &parseWarnings;
}



void EpiMCMCConfig::save(string& filename) {
  
  /** Writes the config DOM out to disc **/
  
}



set<string>* EpiMCMCConfig::getParameterNames()
{
  /** Returns a pointer to an STL set of parameter names.
  Throws a ConfigException if duplicate names are found in the DOM **/
  
  XMLCh* buff;  // Character buffer.  Used to delete objects mainly.
  
  buff = XMLString::transcode("parameter");
  DOMNodeList* parmList = doc->getElementsByTagName(buff);
  XMLString::release(&buff);
  
  set<string>* parmNames = new set<string>;
  pair<set<string>::iterator,bool> isInserted;
  
  for(size_t i = 0; i<parmList->getLength(); ++i) {
    DOMNode* parm = parmList->item(i);
    
    buff = XMLString::transcode("id");
    DOMNode* idAttr = parm->getAttributes()->getNamedItem(buff);
    XMLString::release(&buff);
    
    char* id = XMLString::transcode(idAttr->getNodeValue()) ;
    isInserted = parmNames->insert(id);
    if(isInserted.second == false) {
      string msg("Duplicate string inserted: ");
      msg += id;
      XMLString::release(&id);
      throw ConfigException(msg.c_str());
    }
    XMLString::release(&id);
  }
  
  return parmNames;
  
}



ConfigErrorHandler::ConfigErrorHandler(ConfigErrorList& _errors, ConfigWarningList& _warnings) :
    errors(_errors),
    warnings(_warnings)
{
}



void ConfigErrorHandler::error(const SAXParseException& exc)
{
  char* buff = XMLString::transcode(exc.getMessage());
  stringstream ss;
  ss << "Line " << exc.getLineNumber() << ", col " << exc.getColumnNumber() << ": " << buff;
  XMLString::release(&buff);
  
  errors.push_back(ss.str());
}



void ConfigErrorHandler::warning(const SAXParseException& exc)
{
  char* buff = XMLString::transcode(exc.getMessage());
  stringstream ss;
  ss << "Line " << exc.getLineNumber() << ", col " << exc.getColumnNumber() << ": " << buff;
  XMLString::release(&buff);
  
  warnings.push_back(ss.str());
}



void ConfigErrorHandler::fatalError(const SAXParseException& exc)
{
  throw exc;
}



void ConfigErrorHandler::resetErrors()
{
  errors.clear();
  warnings.clear();
}



ParmData::ParmData(const PRIORDIST& _prior, const map<string,double> hp) :
    priordist(_prior),
    hyperParms(hp),
    mhTuning(1.0),
    startVal(1.0)
{
  
}



void ParmData::setMHTuning(const double& val)
{
  mhTuning = val;
}



void ParmData::setStartVal(const double& val)
{
  startVal = val;
  std::cout << __PRETTY_FUNCTION__ << " (" << this << "): " << val << std::endl;
}



PRIORDIST ParmData::getPriorDist()
{
  return priordist;
}



map<string,double>* ParmData::getHyperParms()
{
  return &hyperParms;
}



double ParmData::getMHTuning()
{
  return mhTuning;
}



double ParmData::getStartVal()
{
  std::cout << __PRETTY_FUNCTION__ << " (" << this << "): " << startVal << std::endl;
  return startVal;
}
