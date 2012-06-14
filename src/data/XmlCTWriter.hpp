/* ./src/data/XmlCTWriter.hpp
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

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Name: XmlCTWriter                                                //
// Author: C.Jewell                                                 //
// Created: 23/11/2007                                              //
// Purpose: Implements a DOM to hold, and write to disk, simulated  //
//           contact tracing data                                   //
// Requirements: Xerces-c XML parsing library                       //
//                                                                  //
//////////////////////////////////////////////////////////////////////



#ifndef XMLCTWRITER_HPP
#define XMLCTWRITER_HPP


// Generic includes
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <stdexcept>


// Xerces headers
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/dom/DOMDocumentTraversal.hpp>

#if defined(XERCES_NEW_IOSTREAMS)
#include <iostream>
#else
#include <iostream.h>
#endif
#define _UNICODE 1


// Namespaces

XERCES_CPP_NAMESPACE_USE
using namespace std;



// Transcoding macros
#define UNI(str) XStr(str).unicodeForm()
#define UNINUM(num) XNum(num).unicodeForm()



// DTD macros
#define DTD_PUBLIC_ID "contact-tracing-20070517.dtd"
#define DTD_URI "http://www.maths.lancs.ac.uk/~jewellc/files/contact-tracing-20070517.dtd"



// Typedefs


// Class decls

class XmlCTData
{
private:
  DOMDocument* domFactory;
  double myStartTime;
  DOMElement* ctData;

protected:

  bool hasStartTime;
  bool hasID;

public:
  
  XmlCTData(DOMDocument*);
  ~XmlCTData();
  void setCTStartTime(const double time);
  void setID(const size_t id);
  size_t getID();
  void truncate();
  void appendContact(const size_t _id,
		     const bool _incoming,
		     const char* _type,
		     const double _time,
		     const bool _caused = false); 
  bool hasContacts();

  DOMElement* getCtData(); // Returns CT Data
  void attachCtData(const DOMElement* toAttach); // Attaches existing ctdata
  void detachCtData(); // Resets the DOM

  void dumpNode(const DOMNode* toDump);

  friend class XmlCTWriter;
};



class XmlCTWriter {

private:
  DOMImplementation* impl;
  DOMDocumentType* myDoctype;
  DOMDocument* ctDoc;
  DOMElement* rootElem;
  DOMLSParser* parser;

public:
  XmlCTWriter();
  ~XmlCTWriter();
  XmlCTData* createCTData(const size_t label);
  void addCTData(XmlCTData* const ctData);
  bool hasContacts();
  void resetDOM();
  void writeToFile(const string filename);
  void readFromFile(const string filename, map<int, XmlCTData*> ctData);




};




// XML Helper classes

class XStr {
public:
  XStr(const char* const);
  ~XStr();
  const XMLCh* unicodeForm() const;

private:

  XMLCh* fUnicodeForm;
};



class XNum {
  // Converts numbers (doubles and ints) to 
  // XMLCh* format
public:
  XNum(const size_t&);
  XNum(const double&);
  ~XNum();
  const XMLCh* unicodeForm() const;

private:
  char buffer[100];
  XMLCh* fUnicodeForm;
};



#endif
