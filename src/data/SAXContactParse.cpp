/* ./src/data/SAXContactParse.cpp
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

#include "SAXContactParse.hpp"



// -----------------------------------------------------------------------
// Handler methods 
// -----------------------------------------------------------------------

ContactSAXHandler::ContactSAXHandler(vector<infection>& individuals) :
  individuals(individuals),
  inFrom(false),
  inTo(false)
{
  // Constructor creates a hash index of labels for the infected vector

  // Build a hash index for the infectives
  vector<infection>::iterator infecIter = individuals.begin();
  while(infecIter != individuals.end()) {
    infecIndex.insert(pair<size_t,vector<infection>::iterator>(infecIter->label,
							       infecIter)
		      );
    infecIter++;
  }

}



ContactSAXHandler::~ContactSAXHandler()
{
}



void ContactSAXHandler::startDocument()
{
  // Sets the start state
  state = START;
}



void ContactSAXHandler::endDocument()
{

  // Does nothing for now
  if(state != FINISH) {
    cerr << "Final state: " << state << endl;
    throw logic_error("EOF before document is complete");
  }
  //XMLString::release(&charBuffer);
  //  cout << "End Document" << endl;
 
}


void ContactSAXHandler::startElement(const XMLCh* const name,
			   AttributeList& attributes)
{
  char* ascName = XMLString::transcode(name);
  XMLString::release(&ascName);

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
      currContactList.clear(); // Start afresh with new contact list
      const XMLCh* id = attributes.getValue("id");
      currInfective = findId(atoi(XMLString::transcode(id)));
      const XMLCh* start = attributes.getValue("start");
      currConStart = atof(XMLString::transcode(start));
    }
    else
      throw logic_error("XML Parser error: garbled XML input at <contact>");
    break;

  case FROM:
    if(state == CONTACT) {
      state = FROM;
      inFrom = true;
      const XMLCh* id = attributes.getValue("id");
      ascName = XMLString::transcode(id);
      currSource = findId(atoi(ascName));
      XMLString::release(&ascName);
    }
    else
      throw logic_error("XML Parser error: garbled XML input at <from>");
    break;

  case TO:
    if(state == CONTACT) {
      state = TO;
      inTo = true;
      const XMLCh* id = attributes.getValue("id");
      ascName = XMLString::transcode(id);
      currSource = findId(atoi(ascName));
      XMLString::transcode(id);
    }
    else 
      throw logic_error("XML Parser error: garbled XML input at <to>");
    break;

  case TYPE:
    if(state == FROM | state == TO)
      state = TYPE;
    else {
      cerr << "State at <type>:" << state << endl;
      throw logic_error("XML Parser error: garbled XML input at <type>");
    }
    break;

  case TIME:
    if(state == FROM | state == TO)
      state = TIME;
    else
      throw logic_error("XML Parser error: garbled XML input at <time>");
    break;

  }
}



void ContactSAXHandler::endElement(const XMLCh* const name)
{
  char* ascName = XMLString::transcode(name);
  XMLString::release(&ascName);

  try{
    switch(enumState(name)) {
      
    case TRACEDCONTACTS:
      if(state == TRACEDCONTACTS)
	state = FINISH;
      else 
	throw logic_error("XML Parser error: garbled XML input at </tracedcontacts>");
      //cout << "Leaving TRACEDCONTACTS" << endl;
      break;
      
    case CONTACT:
      //cout << "Leaving CONTACT" << endl;
      if(state == CONTACT) {
	setContactStart(); // Commit the parsed contacts
	state = TRACEDCONTACTS;
      }
      else
	throw logic_error("XML Parser error: garbled XML input at </contact>");
      break;
      
    case FROM:
      if(state == FROM) {
	addFromContact();
	state = CONTACT;
        inFrom = false;
      }
      else
	throw logic_error("XML Parser error: garbled XML input at </from>");
      break;

    case TO:
      if(state == TO) {
	addToContact();
	state = CONTACT;
        inTo = false;
      }
      else
	throw logic_error("XML Parser error: garbled XML input at </to>");
      break;
      
    case TYPE:
      if(state == TYPE) {
	if(XMLString::stringLen(charBuffer) == 0) {
	  cerr << "Null string in TYPE! State: " << state << endl;
	  cerr << "To: " << inTo << endl;
	  cerr << "Id: " << currInfective->label << endl;
	  cerr << "Source: " << currSource->label << endl;
	}
	currType = enumConType(charBuffer);
	XMLString::release(&charBuffer);
	if(inFrom) state = FROM;
	else if(inTo) state = TO;
	else throw logic_error("No FROM or TO state set!");
      }    
      else
	throw logic_error("XML Parser error: garbled XML input at </type>");
      break;
      
    case TIME:
      if(state == TIME) {
	currTime = atof(XMLString::transcode(charBuffer));
	XMLString::release(&charBuffer);
	if(inFrom) state = FROM;
	else if(inTo) state = TO;
	else throw logic_error("No FROM or TO state set in TIME!");
      }
      else
	throw logic_error("XML Parser error: garbled XML input at </time>");
      break;

    default:
      throw logic_error("XML Parser error: Unrecognised tag");
      
    }
  }
  catch(exception& e) {
    XMLString::release(&charBuffer);
    throw logic_error(e.what());
  }

  //cout << "Leaving end element" << endl;


}



void ContactSAXHandler::characters(const XMLCh* const chars,
				   const XMLSize_t length)
{
  // Dynamically allocate a char array to hold our characters
  //cout << "Characters...";
  charBuffer = XMLString::replicate(chars);
}



infection* ContactSAXHandler::findId(size_t index)
{
  // Sets the iterator to the current infected label
  map<size_t,vector<infection>::iterator>::iterator search;

  search = infecIndex.find(index);

  if(search == infecIndex.end())
    throw logic_error("ContactSAXHandler::setId(size_t): id not found in infection vector");

  return &(*search->second);
}



XMLState ContactSAXHandler::enumState(const XMLCh* const ascState)
{
  // Enumerates the tag name


  if(XMLString::compareIString(ascState,XMLString::transcode("tracedcontacts"))==0) {
    return TRACEDCONTACTS;
  }
  else if(XMLString::compareIString(ascState,XMLString::transcode("contact"))==0) {
    return CONTACT;
  }
  else if(XMLString::compareIString(ascState,XMLString::transcode("from"))==0) {
    return FROM;
  }
  else if(XMLString::compareIString(ascState,XMLString::transcode("to"))==0) {
    return TO;
  }
  else if(XMLString::compareIString(ascState,XMLString::transcode("type"))==0) {
    return TYPE;
  }
  else if(XMLString::compareIString(ascState,XMLString::transcode("time"))==0) {
    return TIME;
  }
  else {
    cerr << XMLString::transcode(ascState) << endl; 
    throw logic_error("Bad XMLState");
  }
}



CON_e ContactSAXHandler::enumConType(const XMLCh* const ascType)
{
  // This turns an ascii representation
  // of the contact type (from input file)
  // into type CON_e

  if(XMLString::compareIString(ascType,XMLString::transcode("feedmill")) == 0) {
    return FEEDMILL;
  }
  else if (XMLString::compareIString(ascType,XMLString::transcode("shouse"))==0) {   
    return SHOUSE;
  }
  else if (XMLString::compareIString(ascType,XMLString::transcode("company"))==0) {
    return COMPANY;
  }
  else {
    cerr << "\"" << XMLString::transcode(ascType) << "\"" << endl;
    throw logic_error("Cannot set contact type");
  }
}



void ContactSAXHandler::addFromContact()
{
  // Adds the latest contact to the growing current contact list
  currInfective->contacts.insert(Contact(currSource,currType,currTime));
}



void ContactSAXHandler::addToContact()
{
  // Adds the latest contact to someone else's contact list
  currSource->contacts.insert(Contact(currInfective,currType,currTime));
}



void ContactSAXHandler::setContactStart()
{
  // Copies a contact list into an infection object

  currInfective->contactStart = currConStart; 
  //attachContacts(currContactList,currConStart);
}



void ContactSAXHandler::error(const SAXParseException& e)
{
  //throw SAXParseException(e);
  cerr << "Error: " << XMLString::transcode(e.getMessage()) << endl;
}



void ContactSAXHandler::warning(const SAXParseException& e)
{
  throw SAXParseException(e);
}



void ContactSAXHandler::fatalError(const SAXParseException& e)
{
  throw SAXParseException(e);
}



// ------------------------------------------------------------------------
// Main function
// ------------------------------------------------------------------------

void SAXContactParse(const char* const filename, vector<infection>& individuals)
{
  // This function parses a contact xml file into a sinrEpi::infection vector

  try {
    XMLPlatformUtils::Initialize();
  }
  catch (const XMLException& e) {
    char* message = XMLString::transcode(e.getMessage());
    cerr << "Error during XML parser initialisation:\n\t"
	 << message << "\n";
    XMLString::release(&message);
    throw logic_error("Initialisation error");
  }

  SAXParser* parser = new SAXParser();

  parser->setValidationScheme(SAXParser::Val_Never);
  parser->setDoNamespaces(false);
  parser->setValidationConstraintFatal(false);


  int errorCode = 0;
  int errCount = 0;

  try{
    ContactSAXHandler* docHandler = new ContactSAXHandler(individuals);
    SAXContactErrorReporter* errorReporter = new SAXContactErrorReporter;
    parser->setDocumentHandler(docHandler);
    parser->setErrorHandler(errorReporter);
    //cout << "Starting parse" << endl;
    parser->parse(filename);
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

  XMLPlatformUtils::Terminate();

  if(errCount > 0) {
    cerr << errCount << " XML parse errors present.  Cannot continue" << endl;
    throw logic_error("Fatal error: XML parse errors present");
  }

}



/////////////////////////////////////////////////////////////////
// Error handler
/////////////////////////////////////////////////////////////////

inline bool SAXContactErrorReporter::getSawErrors() const
{
    return fSawErrors;
}



void SAXContactErrorReporter::warning(const SAXParseException&)
{
    //
    // Ignore all warnings.
    //
}

void SAXContactErrorReporter::error(const SAXParseException& toCatch)
{
    fSawErrors = true;
    cerr << "Error at file \"" << XMLString::transcode(toCatch.getSystemId())
		 << "\", line " << toCatch.getLineNumber()
		 << ", column " << toCatch.getColumnNumber()
         << "\n   Message: " << XMLString::transcode(toCatch.getMessage()) << endl;
}

void SAXContactErrorReporter::fatalError(const SAXParseException& toCatch)
{
    fSawErrors = true;
    cerr << "Fatal Error at file \"" << XMLString::transcode(toCatch.getSystemId())
		 << "\", line " << toCatch.getLineNumber()
		 << ", column " << toCatch.getColumnNumber()
         << "\n   Message: " << XMLString::transcode(toCatch.getMessage()) << endl;
}

void SAXContactErrorReporter::resetErrors()
{
    fSawErrors = false;
}
