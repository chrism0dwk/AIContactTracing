/* ./src/data/SAXContactParse.hpp
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

#ifndef INCLUDE_SAXCONTACTPARSE_HPP
#define INCLUDE_SAXCONTACTPARSE_HPP

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>

#include <string.h>
#if defined(XERCES_NEW_IOSTREAMS)
#include <iostream>
#else
#include <iostream.h>
#endif
#include <stdlib.h>

#include <vector>
#include <list>
#include <map>

#include "aiTypes.hpp"
#include "sinrEpi.h"


XERCES_CPP_NAMESPACE_USE



enum XMLState {
  // Describes the position of the the parser in the XML
  START,
  TRACEDCONTACTS,
  CONTACT,
  FROM,
  TO,
  TYPE,
  TIME,
  FINISH
};



class ContactSAXHandler : public HandlerBase
{
public:
  ContactSAXHandler(vector<infection>&);
  ~ContactSAXHandler();
  void startDocument();
  void endDocument();
  void startElement(const XMLCh* const,
		    AttributeList&);
  void endElement(const XMLCh* const);
  void characters(const XMLCh*, const XMLSize_t);

  void error(const SAXParseException& e);
  void warning(const SAXParseException& e);
  void fatalError(const SAXParseException& e);

private:
  // ---------------------------------------------------------------
  // Private data members
  // ---------------------------------------------------------------

  vector<infection>& individuals;
  infection* currInfective; // ptr to current infective
  infection* currSource;

  eventTime_t currConStart;
  CON_e currType;
  eventTime_t currTime;
  XMLState state;
  bool inFrom;
  bool inTo;
  XMLCh* charBuffer;

  map<size_t,vector<infection>::iterator> infecIndex; // Hash index
  list<Contact> currContactList; // The current list of contacts
  

  // --------------------------------------------------------------
  // Private function members
  // --------------------------------------------------------------

  // Enumerators (translate ASCII to machine-speak)
  CON_e enumConType(const XMLCh* const);
  XMLState enumState(const XMLCh* const);

  // Event handlers for constructing contact lists
  infection* findId(size_t);
  void setContactStart();
  void addFromContact();
  void addToContact();

};



class SAXContactErrorReporter : public ErrorHandler
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    SAXContactErrorReporter() :
       fSawErrors(false)
    {
    }

    ~SAXContactErrorReporter()
    {
    }


    // -----------------------------------------------------------------------
    //  Implementation of the error handler interface
    // -----------------------------------------------------------------------
    void warning(const SAXParseException& toCatch);
    void error(const SAXParseException& toCatch);
    void fatalError(const SAXParseException& toCatch);
    void resetErrors();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    bool getSawErrors() const;

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fSawErrors
    //      This is set if we get any errors, and is queryable via a getter
    //      method. Its used by the main code to suppress output if there are
    //      errors.
    // -----------------------------------------------------------------------
    bool    fSawErrors;
};



// Wrapper function for populating a sinrEpi::infected vector with contact data

void SAXContactParse(const char* const, vector<infection>&);


#endif
