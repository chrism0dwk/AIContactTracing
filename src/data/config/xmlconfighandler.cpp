/* ./src/data/config/xmlconfighandler.cpp
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
 *   Copyright (C) 2008 by Chris Jewell   *
 *   c.jewell@lancaster.ac.uk   *
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
#include "xmlconfighandler.h"
#include "dataExceptions.h"

XMLConfigHandler::XMLConfigHandler(EpiMCMCConfig& _config) :
    config(_config), parseErrors(0)
{
  // Associate the handler with the config data
}


XMLConfigHandler::~XMLConfigHandler()
{
}


void XMLConfigHandler::load(string& filename)
{
  // Loads config info into the data structure
  try {
    XMLPlatformUtils::Initialize();
  }
  catch (const XMLException&e) {
    throw ConfigParserException(XMLString::transcode(e.getMessage()));
  }
  
  SAXParser* parser = new SAXParser();
  
  parser->setValidationScheme(SAXParser::Val_Always);
  parser->setValidationSchemaFullChecking(true);
  parser->setValidationConstraintFatal(false);
  
  
  int errorCode = 0;
  int errCount = 0;
  parseErrors = 0;
  
  try {
    parser->setDocumentHandler((DocumentHandler*)this);
    parser->setErrorHandler(this);
    
    parser->parse(filename.c_str());
  }
  catch (const OutOfMemoryException&) {
    errorCode = 1;
  }
  catch (const XMLException& e) {
    throw ConfigXmlException(XMLString::transcode(e.getMessage()));
  }
  catch (const SAXParseException& e) {
    throw ConfigParserException(XMLString::transcode(e.getMessage()));
  }
  
  XMLPlatformUtils::Terminate();
  
}



void XMLConfigHandler::save(string& filename)
{
  /** Serialises the config data into a config XML file **/
}



void XMLConfigHandler::startDocument()
{
  
}



void XMLConfigHandler::endDocument()
{
  
}



void XMLConfigHandler::startElement(const XMLCh* const,AttributeList&)
{
  
}



void XMLConfigHandler::endElement(const XMLCh* const)
{
  
}



void XMLConfigHandler::characters(const XMLCh*, const unsigned int)
{
  
  
}



void XMLConfigHandler::error(const SAXParseException& e)
{
  
}



void XMLConfigHandler::warning(const SAXParseException& e)
{
  
}



void XMLConfigHandler::fatalError(const SAXParseException& e)
{
  
}
