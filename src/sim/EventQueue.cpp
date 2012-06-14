/* ./src/sim/EventQueue.cpp
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
 * Event.cpp
 *
 *  Created on: 5 Jan 2010
 *      Author: stsiab
 */

#include <cstdlib>
#include <stdexcept>
#include <iostream>
#include <iomanip>

#include "EventQueue.hpp"

#include "EventParser.hpp"

using namespace std;
using namespace xercesc;

XERCES_CPP_NAMESPACE_USE



Event::Event(const size_t id_, const double time_) : id(id_), time(time_)
{
  // TODO Auto-generated constructor stub

}

Event::~Event()
{
  // TODO Auto-generated destructor stub
}



ContactEvent::ContactEvent(const size_t id_,
                           const double time_,
                           const size_t from_,
                           const ContactType type_) : Event(id_,time_),
                                                      from(from_),
                                                      type(type_)
{

}



bool ContactQueueComp::operator()(const ContactEvent* lhs, const ContactEvent* rhs) const
{
  return lhs->time < rhs->time;
}




ContactQueue::ContactQueue(const string filename)
{
  // Takes an XML file and reads in contact event data
  //  putting it into a sorted list

  try {
    XMLPlatformUtils::Initialize();
  }
  catch (const XMLException& e) {
    cerr << "Could not initialize XML framework. Bailing...";
    abort();
  }


  // Set up the SAX parser
  SAXParser* parser = new SAXParser();

  parser->setValidationScheme(SAXParser::Val_Never);
  parser->setDoNamespaces(false);
  parser->setValidationConstraintFatal(false);


  int errorCode = 0;
  int errCount = 0;

  EventParser* docHandler;
  EventParserErrorReporter* errorReporter;

  try{
    docHandler = new EventParser(eventQueue);
    errorReporter = new EventParserErrorReporter;
    parser->setDocumentHandler(docHandler);
    parser->setErrorHandler(errorReporter);
    //cout << "Starting parse" << endl;
    parser->parse(filename.c_str());
    //cout << "Ending parse" << endl;
  }
  catch (const OutOfMemoryException&) {
    errorCode = 1;
  }
  catch (const XMLException& e) {
    char* message = XMLString::transcode(e.getMessage());
    cerr << "XML Exception: " << message << "\n";
    XMLString::release(&message);
    errorCode = 2;
  }
  catch(const SAXParseException& e) {
    char* message = XMLString::transcode(e.getMessage());
    cerr << "Parse exception: " << message << endl;
    XMLString::release(&message);
    errorCode = 3;
  }

  if(errorCode) {
    XMLPlatformUtils::Terminate();
    if(errorCode == 1) throw bad_alloc();
    else if(errorCode == 2) throw logic_error("XML parse error");
    else if(errorCode == 3) throw logic_error("XML fatal parse error - check your XML");
    else throw logic_error("Unknown error in XML parse");
  }

  //cout << "Terminating..." << endl;

  delete parser;
  delete errorReporter;
  delete docHandler;
  XMLPlatformUtils::Terminate();

  if(errCount > 0) {
    cerr << errCount << " XML parse errors present.  Cannot continue" << endl;
    throw logic_error("Fatal error: XML parse errors present");
  }

  XMLPlatformUtils::Terminate();

  // Set the current event iterator
  this->reset();

  cerr << "Received " << eventQueue.size() << " events in total." << endl;

}



ContactQueue::~ContactQueue()
{
  // Destroy event objects

  EventQueue::iterator iter = eventQueue.begin();
  while(iter != eventQueue.end()) {
    delete *iter;
    iter++;
  }

}



void ContactQueue::dumpEvents()
{
  // Dumps events to stdout

  EventQueue::iterator iter = eventQueue.begin();
  while(iter != eventQueue.end()) {
    cerr << (*iter)->from << " -> " << (*iter)->id << " via method " << (*iter)->type << " at time " << (*iter)->time << endl;
    iter++;
  }
}



void ContactQueue::reset()
{
  //! Resets the current event iterator to the beginning of the event queue
  itCurrEvent = eventQueue.begin();
}



ContactEvent* ContactQueue::next()
{
  //! Returns a pointer to the next event in the queue

  if (itCurrEvent == eventQueue.end()) return NULL;

  ContactEvent* rv = *itCurrEvent;
  itCurrEvent++;

  return rv;
}
