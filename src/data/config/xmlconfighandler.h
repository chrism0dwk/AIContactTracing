/* ./src/data/config/xmlconfighandler.h
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
 *   Copyright (C) 2008 by Chris Jewell                                    *
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
#ifndef XMLCONFIGHANDLER_H
#define XMLCONFIGHANDLER_H

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/XMLFormatter.hpp>

#if defined(XERCES_NEW_IOSTREAMS)
#include <iostream>
#else
#include <iostream.h>
#endif
#include <stdlib.h>

#include <map>
#include <string>

#include "config.h"


XERCES_CPP_NAMESPACE_USE;

/**
XMLConfigHandler provides the code between XML epiRisk config files and the EpiMCMCConfig class.

	@author Chris Jewell <chris.jewell@warwick.ac.uk>
*/



class XMLConfigHandler 
  {
  public:
    XMLConfigHandler(EpiMCMCConfig& _config);
    void load(string& filename);
    void save(string& filename);
    ~XMLConfigHandler();

  private:

    enum XMLState
    {
      // Describes the position of the the parser in the XML file
      START,
      TRACEDCONTACTS,
      CONTACT,
      FROM,
      TYPE,
      TIME,
      FINISH
    };

    EpiMCMCConfig& config;
    
    int parseErrors;
    XMLState state;

    void startDocument();
    void endDocument();
    void startElement(const XMLCh* const,
                      AttributeList&);
    void endElement(const XMLCh* const);
    void characters(const XMLCh*, const unsigned int);

    void error(const SAXParseException& e);
    void warning(const SAXParseException& e);
    void fatalError(const SAXParseException& e);

  };

#endif
