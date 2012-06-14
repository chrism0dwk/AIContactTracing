/* ./src/sim/EventDom.cpp
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

#include "Event.hpp"

// XERCES INCLUDES
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>

#include "DOMErrorHandler.hpp"

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

  // READ IN DATA FROM FILE
  cout << "Parsing XML file '" << filename.c_str() << "'..." << flush;
  XMLCh tempStr[100];
  XMLString::transcode("LS", tempStr, 99);
  DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(tempStr);
  DOMLSParser* parser = ((DOMImplementationLS*)impl)->createLSParser(DOMImplementationLS::MODE_SYNCHRONOUS, 0);

  EpiDOMErrorHandler* errorHandler = new EpiDOMErrorHandler();
  parser->getDomConfig()->setParameter(XMLUni::fgDOMErrorHandler, errorHandler);

  DOMDocument* doc = 0;

  try {
    doc = parser->parseURI(filename.c_str());
  }
  catch (const XMLException& toCatch) {
    char* message = XMLString::transcode(toCatch.getMessage());
    cout << "Exception message is: \n"
         << message << "\n";
    XMLString::release(&message);
    throw runtime_error("XMLException parsing XML file");
  }
  catch (const DOMException& toCatch) {
    char* message = XMLString::transcode(toCatch.msg);
    cout << "Exception message is: \n"
         << message << "\n";
    XMLString::release(&message);
    throw runtime_error("DOMException parsing XML file");
  }
  catch (...) {
    cout << "Unexpected Exception \n" ;
    throw runtime_error("Unexpected exception parsing XML file");
  }

  cout << "Done" << endl;

  cout << "Constructing event queue..." << flush;

  // Walk the DOM and construct the event list.
  XMLSize_t nTotal;
  double tempTime;
  size_t tempLabel, tempFrom;
  ContactType tempType;
  char buff[100];
  // use XMLCh tempStr[100] from above

  XMLCh* idTag = XMLString::transcode("id");
  XMLCh* fromTag = XMLString::transcode("from");
  XMLCh* contactTag = XMLString::transcode("contact");
  XMLCh* typeTag = XMLString::transcode("type");
  XMLCh* timeTag = XMLString::transcode("time");

  DOMElement* root = doc->getDocumentElement();
  DOMNodeList* contactees = root->getElementsByTagName(contactTag);
  nTotal = contactees->getLength();

  DOMNode* currIndiv;
  DOMNode* currContact;
  DOMNode* node;
  DOMNodeList* contacts;

  cout << "Ntotal = " << nTotal << endl;

  for(XMLSize_t j=0; j<nTotal; j++) {

    currIndiv = contactees->item(j);

    XMLString::transcode(((DOMElement*)currIndiv)->getAttribute(idTag),buff,99);
    tempLabel = atoi(buff);

    contacts = ((DOMElement*)currIndiv)->getElementsByTagName(fromTag);
    for(XMLSize_t k=0; k < contacts->getLength(); k++) {

      currContact = contacts->item(k);

      XMLString::transcode(((DOMElement*)currContact)->getAttribute(idTag),buff,99);
      tempFrom = atoi(buff);
      //cout << "ID (Native): " << buff << endl;

      // Get time
      node = ((DOMElement*)currContact)->getElementsByTagName(timeTag)->item(0);
      XMLString::transcode(node->getFirstChild()->getNodeValue(),buff,99);
      tempTime = atof(buff);
      //cout << "Time (Native): " << tempTime << endl;

      // Get type
      node = ((DOMElement*)currContact)->getElementsByTagName(typeTag)->item(0);
      XMLString::transcode(node->getFirstChild()->getNodeValue(),buff,99);
      if (XMLString::equals(buff,"background")) tempType = BACKGROUND;
      else if(XMLString::equals(buff,"feedmill")) tempType = FEEDMILL;
      else if(XMLString::equals(buff,"shouse")) tempType = SHOUSE;
      else if(XMLString::equals(buff,"company")) tempType = COMPANY;
      else if(XMLString::equals(buff,"ispatial")) tempType = ISPATIAL;
      else if(XMLString::equals(buff,"nspatial")) tempType = NSPATIAL;
      else cerr << "Unrecognised contact type '" << buff << "'" << endl;

      //cout << "Type (Native): " << buff << endl;

      // Now add the contact to the event list
      ContactEvent* event = new ContactEvent(tempLabel,tempTime,tempFrom,tempType);

      pair<EventQueue::iterator,bool> rv = eventQueue.insert(event);
      if (rv.second == false) {
        cerr << setprecision(8) << "Duplicate time found in contact XML:" << tempTime << endl;
      }

    }
  }

  XMLString::release(&idTag);
  XMLString::release(&fromTag);
  XMLString::release(&contactTag);
  XMLString::release(&timeTag);
  XMLString::release(&typeTag);

  parser->release();
  delete errorHandler;

  XMLPlatformUtils::Terminate();

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
