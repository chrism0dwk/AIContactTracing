/* ./src/sim/EventParser.hpp
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
 * EventParser.hpp
 *
 *  Created on: 5 Jan 2010
 *      Author: stsiab
 */

#ifndef EVENTPARSER_HPP_
#define EVENTPARSER_HPP_

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


#include "EventQueue.hpp"



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



class EventParser : public HandlerBase
{
public:
  EventParser(EventQueue& eventQueue_);
  virtual
  ~EventParser();
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

  EventQueue& eventQueue;

  ContactEvent* currEvent;
  size_t contactee;
  size_t contactor;
  double eventTime;
  ContactType eventType;

  XMLState state;
  bool inFrom;
  bool inTo;
  XMLCh xmlBuff[100];
  char chBuff[100];
  XMLCh* charBuffer;

  //TAGS
  XMLCh* TAG_tracedcontacts;
  XMLCh* TAG_contact;
  XMLCh* TAG_from;
  XMLCh* TAG_to;
  XMLCh* TAG_type;
  XMLCh* TAG_time;


  // --------------------------------------------------------------
  // Private function members
  // --------------------------------------------------------------

  // Enumerators (translate ASCII to machine-speak)
  XMLState enumState(const XMLCh* const);

  // Event handlers for constructing contact lists
  void setContactStart();
  void addFromContact();
  void addToContact();

};



class EventParserErrorReporter : public ErrorHandler
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    EventParserErrorReporter() :
       fSawErrors(false)
    {
    }

    ~EventParserErrorReporter()
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


#endif /* EVENTPARSER_HPP_ */
