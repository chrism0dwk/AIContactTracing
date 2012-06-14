/* ./src/sim/EventParser.cpp
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
 * EventParser.cpp
 *
 *  Created on: 5 Jan 2010
 *      Author: stsiab
 */


#include <stdexcept>
#include <iostream>
#include <cassert>

#include "EventParser.hpp"

// -----------------------------------------------------------------------
// Handler methods
// -----------------------------------------------------------------------

EventParser::EventParser(EventQueue& eventQueue_) :
  eventQueue(eventQueue_), charBuffer(NULL)
{

  TAG_tracedcontacts = XMLString::transcode("tracedcontacts");
  TAG_contact = XMLString::transcode("contact");
  TAG_from = XMLString::transcode("from");
  TAG_to = XMLString::transcode("to");
  TAG_type = XMLString::transcode("type");
  TAG_time = XMLString::transcode("time");
}



EventParser::~EventParser()
{
  XMLString::release(&TAG_tracedcontacts);
  XMLString::release(&TAG_contact);
  XMLString::release(&TAG_from);
  XMLString::release(&TAG_to);
  XMLString::release(&TAG_type);
  XMLString::release(&TAG_time);
}



void EventParser::startDocument()
{
  // Sets the start state
  state = START;
}



void EventParser::endDocument()
{
  // Does nothing for now
  if(state != FINISH) {
    cerr << "Final state: " << state << endl;
    throw logic_error("EOF before document is complete");
  }
  //XMLString::release(&charBuffer);
  //  cout << "End Document" << endl;

}


void EventParser::startElement(const XMLCh* const name,
                           AttributeList& attributes)
{
  //char* ascName = XMLString::transcode(name);
  //cout << "State (ASC): " << ascName << endl;
  //XMLString::release(&ascName);

  if (charBuffer != NULL) XMLString::release(&charBuffer);

  switch(enumState(name)) {

  case TRACEDCONTACTS:
    //cout << "Entered TRACEDCONTACTS" << endl;
    if(state == START)
      state = TRACEDCONTACTS;
    else
      throw logic_error("XML Parser error: garbled XML input at <tracedcontacts>");
    break;

  case CONTACT:
    //cout << "Entered CONTACT" << endl;
    if(state == TRACEDCONTACTS) {
      state = CONTACT;

      const XMLCh* id = attributes.getValue("id");
      XMLString::transcode(id,chBuff,99);
      contactee = atoi(chBuff);
    }
    else
      throw logic_error("XML Parser error: garbled XML input at <contact>");
    break;

  case FROM:
    if(state == CONTACT) {
      state = FROM;
      inFrom = true;
      const XMLCh* id = attributes.getValue("id");
      XMLString::transcode(id,chBuff,99);
      contactor = atoi(chBuff);
    }
    else
      throw logic_error("XML Parser error: garbled XML input at <from>");
    break;

  case TYPE:
    if(state == FROM)
      state = TYPE;
    else {
      cerr << "State at <type>:" << state << endl;
      throw logic_error("XML Parser error: garbled XML input at <type>");
    }
    break;

  case TIME:
    assert(state == FROM);
    if(state == FROM)
      state = TIME;
    else
      throw logic_error("XML Parser error: garbled XML input at <time>");
    break;

  default:
    throw logic_error("Unknown state in parse");

  }
}



void EventParser::endElement(const XMLCh* const name)
{

  try{
    switch(enumState(name)) {

    case TRACEDCONTACTS:
      if(state == TRACEDCONTACTS)
        state = FINISH;
      else {
        XMLString::release(&charBuffer);
        throw logic_error("XML Parser error: garbled XML input at </tracedcontacts>");
      }

      //cout << "Leaving TRACEDCONTACTS" << endl;
      break;

    case CONTACT:
      //cout << "Leaving CONTACT" << endl;
      if(state == CONTACT) {
         state = TRACEDCONTACTS;
      }
      else {
        XMLString::release(&charBuffer);
        throw logic_error("XML Parser error: garbled XML input at </contact>");
      }
      break;

    case FROM:
      if(state == FROM) {
        currEvent = new ContactEvent(contactee,eventTime,contactor,eventType);
        pair<EventQueue::iterator, bool> rv = eventQueue.insert(currEvent);
        if(rv.second == false) {
          cerr << "Duplicate time: " << eventTime << endl;
        }
        state = CONTACT;
        inFrom = false;
      }
      else {
        cerr << "State: " << state << endl;
        XMLString::release(&charBuffer);
        throw logic_error("XML Parser error: garbled XML input at </from>");
      }
      break;

    case TYPE:
      if(state == TYPE) {
        if(XMLString::stringLen(charBuffer) == 0) {
          cerr << "Warning: Type empty!" << endl;
        }

        XMLString::transcode(charBuffer,chBuff,99);

        if (XMLString::equals(chBuff,"background")) eventType = BACKGROUND;
        else if(XMLString::equals(chBuff,"feedmill")) eventType = FEEDMILL;
        else if(XMLString::equals(chBuff,"shouse")) eventType = SHOUSE;
        else if(XMLString::equals(chBuff,"company")) eventType = COMPANY;
        else if(XMLString::equals(chBuff,"ispatial")) eventType = ISPATIAL;
        else if(XMLString::equals(chBuff,"nspatial")) eventType = NSPATIAL;
        else cerr << "Unrecognised contact type '" << chBuff << "'" << endl;

        state = FROM;

      }
      else {
        XMLString::release(&charBuffer);
        throw logic_error("XML Parser error: garbled XML input at </type>");
      }
      break;

    case TIME:
      if(state == TIME) {
        XMLString::transcode(charBuffer,chBuff,99);
        eventTime = atof(chBuff);
        state = FROM;
      }
      else {
        XMLString::release(&charBuffer);
        throw logic_error("XML Parser error: garbled XML input at </time>");
      }
      break;

    default:
      XMLString::release(&charBuffer);
      throw logic_error("XML Parser error: Unrecognised tag");

    }

    XMLString::release(&charBuffer);
  }
  catch(exception& e) {
    XMLString::release(&charBuffer);
    throw logic_error(e.what());
  }

  //cout << "Leaving end element" << endl;


}



void EventParser::characters(const XMLCh* const chars,
                                   const XMLSize_t length)
{
  // Dynamically allocate a char array to hold our characters
  //cout << "Characters...";

  if(charBuffer != NULL) XMLString::release(&charBuffer);
  charBuffer = XMLString::replicate(chars);
}




XMLState EventParser::enumState(const XMLCh* const ascState)
{
  // Enumerates the tag name




  if(XMLString::compareIString(ascState,TAG_tracedcontacts)==0) {
    return TRACEDCONTACTS;
  }
  else if(XMLString::compareIString(ascState,TAG_contact)==0) {
    return CONTACT;
  }
  else if(XMLString::compareIString(ascState,TAG_from)==0) {
    return FROM;
  }
  else if(XMLString::compareIString(ascState,TAG_to)==0) {
    return TO;
  }
  else if(XMLString::compareIString(ascState,TAG_type)==0) {
    return TYPE;
  }
  else if(XMLString::compareIString(ascState,TAG_time)==0) {
    return TIME;
  }
  else {
    char* msg;
    msg = XMLString::transcode(ascState);
    cerr << msg << endl;
    XMLString::release(&msg);
    throw logic_error("Bad XMLState");
  }
}






void EventParser::error(const SAXParseException& e)
{
  //throw SAXParseException(e);
  cerr << "Error: " << XMLString::transcode(e.getMessage()) << endl;
}



void EventParser::warning(const SAXParseException& e)
{
  throw SAXParseException(e);
}



void EventParser::fatalError(const SAXParseException& e)
{
  throw SAXParseException(e);
}






/////////////////////////////////////////////////////////////////
// Error handler
/////////////////////////////////////////////////////////////////

inline bool EventParserErrorReporter::getSawErrors() const
{
    return fSawErrors;
}



void EventParserErrorReporter::warning(const SAXParseException&)
{
    //
    // Ignore all warnings.
    //
}

void EventParserErrorReporter::error(const SAXParseException& toCatch)
{
    fSawErrors = true;
    cerr << "Error at file \"" << XMLString::transcode(toCatch.getSystemId())
                 << "\", line " << toCatch.getLineNumber()
                 << ", column " << toCatch.getColumnNumber()
         << "\n   Message: " << XMLString::transcode(toCatch.getMessage()) << endl;
}

void EventParserErrorReporter::fatalError(const SAXParseException& toCatch)
{
    fSawErrors = true;
    cerr << "Fatal Error at file \"" << XMLString::transcode(toCatch.getSystemId())
                 << "\", line " << toCatch.getLineNumber()
                 << ", column " << toCatch.getColumnNumber()
         << "\n   Message: " << XMLString::transcode(toCatch.getMessage()) << endl;
}

void EventParserErrorReporter::resetErrors()
{
    fSawErrors = false;
}
