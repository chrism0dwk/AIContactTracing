/* ./src/data/XmlCTWriter.cpp
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

#include <sstream>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include "XmlCTWriter.hpp"
#include "EpiRiskException.hpp"
//#include "DOMErrorHandler.hpp"

//! Ctor is created using a DOMDocument factory
XmlCTData::XmlCTData(DOMDocument* _factory) :
  domFactory(_factory), hasStartTime(false), hasID(false), ctData(NULL)
{
  try
    {
      ctData = domFactory->createElement(UNI("contact"));
    }
  catch (const XMLException& e)
    {
      char* msg = XMLString::transcode(e.getMessage());
      cerr << msg << endl;
      XMLString::release(&msg);
      throw runtime_error("Exception occurred in XmlCTData::XmlCTData");
    }
  catch (...)
    {
      throw runtime_error("Unknown exception in XmlCTData::XmlCTData");
    }
}

XmlCTData::~XmlCTData()
{
  DOMNode* parent = ctData->getParentNode();

  if (parent != NULL)
    {
      parent->removeChild(ctData);
    }
  if (ctData != NULL)
    {
      ctData->release();
    }
  else
    {
      cerr << "Not releasing ctData in " << __PRETTY_FUNCTION__ << endl;
    }
}

//! Sets the contact tracing start time
void
XmlCTData::setCTStartTime(const double time)
{
  myStartTime = time;
  try
    {
      ctData->setAttribute(UNI("start"), UNINUM(time));
    }
  catch (const DOMException& e)
    {
      stringstream msg;
      msg << "DOMException code: " << e.code;
      throw runtime_error(msg.str().c_str());
    }
  catch (const XMLException& e)
    {
      throw runtime_error("XMLException setting CT start time");
    }
  catch (const exception& e)
    {
      cerr << e.what() << endl;
      throw runtime_error("Exception setting CT start time");
    }
  catch (...)
    {
      throw runtime_error("Exception setting CT start time");
    }

  hasStartTime = true;
}

//! Sets the id of the receiver
void
XmlCTData::setID(const size_t id)
{
  try
    {
      ctData->setAttribute(UNI("id"), UNINUM(id));
    }
  catch (const XMLException& e)
    {
      throw runtime_error("XMLException setting CT id");
    }
  catch (...)
    {
      throw runtime_error("Exception setting CT id");
    }

  hasID = true;
}

size_t
XmlCTData::getID()
{
  ctData->getAttribute(UNI("id"));

  char* ascString = XMLString::transcode(ctData->getAttribute(UNI("id")));

  size_t label = atoi(ascString);

  XMLString::release(&ascString);

  return label;
}

bool
XmlCTData::hasContacts()
{
  //! Returns true if ctData has contacts

  if (ctData->hasChildNodes())
    return true;
  else
    return false;
}

void
XmlCTData::truncate()
{
  //! Left-truncates the contacts at 
  //!   start time.

  if (!hasStartTime)
    throw logic_error("Cannot truncate: no contact start time specified");

  char* strTime;
  double conTime;
  DOMElement* child;
  DOMElement* item;
  DOMNodeList* contactInfo;
  DOMElement * time;
  const XMLCh* value;
  DOMNodeList* children = ctData->getChildNodes();
  vector<DOMElement*> removeElements;

  //cout << "Num contacts = " << children->getLength() << endl;
  try
    {

      // Find element < myStartTime
      for (int i = 0; i < children->getLength(); ++i)
        {
          item = (DOMElement*) children->item(i);

          if (item->getNodeType() != DOMNode::ELEMENT_NODE) continue;

          contactInfo = item->getElementsByTagName(UNI("time"));
          time = (DOMElement*) contactInfo->item(0);
          value = time->getFirstChild()->getNodeValue();
          strTime = XMLString::transcode(value);
          conTime = atof(strTime);
          XMLString::release(&strTime);

          if (conTime < myStartTime)
            {
              removeElements.push_back(item);
            }
        }

      // Remove them:
      for (int i = 0; i < removeElements.size(); ++i)
        {
          ctData->removeChild(removeElements.at(i));
        }

    }
  catch (DOMException& e)
    {
      cerr << XMLString::transcode(e.getMessage()) << endl;
      throw runtime_error("DOMException in function truncate");
    }
  catch (XMLException& e)
    {
      cerr << XMLString::transcode(e.getMessage()) << endl;
      throw runtime_error("XMLException in function truncate");
    }
  catch (...)
    {
      throw runtime_error("Unknown exception in function truncate");
    }

}

void
XmlCTData::appendContact(const size_t _id, const bool _incoming,
    const char* _type, const double _time, const bool _caused)
{
  // Appends contact data <from> information to ctData

  DOMElement* from;
  DOMElement* type;
  DOMElement * time;
  DOMText* value;

  XMLCh* conType = XMLString::transcode(_type);

  try
    {
      if (_incoming == true)
        from = domFactory->createElement(UNI("from"));
      else
        from = domFactory->createElement(UNI("to"));
      from->setAttribute(UNI("id"), UNINUM(_id));
      if (_caused == true)
        from->setAttribute(UNI("caused"), UNI("true"));

      type = domFactory->createElement(UNI("type"));
      value = domFactory->createTextNode(conType);
      type->appendChild(value);

      time = domFactory->createElement(UNI("time"));
      value = domFactory->createTextNode(UNINUM(_time));
      time->appendChild(value);

      from->appendChild(type);
      from->appendChild(time);

      ctData->appendChild(from);

      XMLString::release(&conType);
    }
  catch (const XMLException& e)
    {
      cerr << XMLString::transcode(e.getMessage()) << endl;
      throw runtime_error(
          "XMLException thrown in XmlCTWriter::appendContact(const XmlCTData*, const size_t, const Contype_e, const double)");
    }
  catch (...)
    {
      throw runtime_error(
          "Unknown exception thrown in XmlCTWriter::appendContact(const XmlCTData*, const size_t, const Contype_e, const double)");
    }
}

DOMElement*
XmlCTData::getCtData()
{
  //! Returns a pointer to the contact tracing data contained here
  return ctData;
}

void
XmlCTData::attachCtData(const DOMElement* toAttach)
{
  //! Attaches pre-existing ct data
  //! Note: No validation of the data occurs here!
  // Performs deep copy of toAttach

  if (ctData != NULL)
    { // Get rid of pre-existing data.
      ctData->release();
    }

  ctData = (DOMElement*) toAttach->cloneNode(true);
}

void
XmlCTData::dumpNode(const DOMNode* toDump)
{
  XMLCh tempStr[100];
  XMLString::transcode("LS", tempStr, 99);
  DOMImplementation* myImpl = DOMImplementationRegistry::getDOMImplementation(tempStr);
  DOMLSSerializer* theSerializer =
      ((DOMImplementationLS*) myImpl)->createLSSerializer();
  if (theSerializer->getDomConfig()->canSetParameter(
      XMLUni::fgDOMWRTFormatPrettyPrint, true))
    theSerializer->getDomConfig()->setParameter(
        XMLUni::fgDOMWRTFormatPrettyPrint, true);

  XMLFormatTarget *myFormTarget = new StdOutFormatTarget();
  DOMLSOutput* theOutput = ((DOMImplementationLS*) myImpl)->createLSOutput();
  theOutput->setByteStream(myFormTarget);

  try
    {
      // do the serialization through DOMLSSerializer::write();
      theSerializer->write(toDump, theOutput);
    }
  catch (const XMLException& toCatch)
    {
      char* message = XMLString::transcode(toCatch.getMessage());
      cout << "Exception message is: \n" << message << "\n";
      XMLString::release(&message);
    }
  catch (const DOMException& toCatch)
    {
      char* message = XMLString::transcode(toCatch.msg);
      cout << "Exception message is: \n" << message << "\n";
      XMLString::release(&message);
    }
  catch (...)
    {
      cout << "Unexpected Exception \n";
    }

  theOutput->release();
  theSerializer->release();
  delete myFormTarget;

}

XmlCTWriter::XmlCTWriter() :
  parser(NULL)
{
  // Initialises the DOM

  try
    {
      XMLPlatformUtils::Initialize();

      impl = DOMImplementation::getImplementation();
      myDoctype = impl->createDocumentType(UNI("tracedcontacts"),
          UNI(DTD_PUBLIC_ID), UNI(DTD_URI));

      ctDoc = impl->createDocument(UNI("tracedcontacts"),
          UNI("tracedcontacts"), myDoctype);
      ctDoc->setXmlStandalone(0);


      rootElem = ctDoc->getDocumentElement();
    }
  catch (const XMLException& e)
    {
      cerr << "Error during XmlCTWriter construction" << endl;
      cerr << "Error was " << XMLString::transcode(e.getMessage()) << endl;
      XMLPlatformUtils::Terminate();
      throw runtime_error("Error during Xerces-c initialisation");
    }
  catch (...)
    {
      XMLPlatformUtils::Terminate();
      throw runtime_error("Exception during Xerces-c initialisation");
    }

}

XmlCTWriter::~XmlCTWriter()
{
  // Destroy the DOM tree
  ctDoc->release();
  if (parser)
    parser->release();
  XMLPlatformUtils::Terminate();
}

XmlCTData*
XmlCTWriter::createCTData(size_t label)
{
  // Returns an object of type XmlCTData which is used to add contacts to

  XmlCTData* myData;

  try
    {
      myData = new XmlCTData(ctDoc);
      myData->setID(label);
    }
  catch (const XMLException& e)
    {
      cerr << XMLString::transcode(e.getMessage()) << endl;
      XMLPlatformUtils::Terminate();
      throw runtime_error(
          "Exception occurred in XmlCTWriter::createCTData(size_t,double)");
    }
  catch (...)
    {
      XMLPlatformUtils::Terminate();
      throw runtime_error(
          "Unknown exception in XmlCTWriter::createCTData(size_t,double)");
    }

  return myData;
}

void
XmlCTWriter::addCTData(XmlCTData* const _Data)
{
  // Adds a <contact> node

  DOMNode* newNode;

  try
    {
      newNode = _Data->getCtData();
    }
  catch (const DOMException& e)
    {
      stringstream msg;
      msg << "Error importing node into output document.  DOM Exception code: " << e.code << endl;
      throw runtime_error(msg.str().c_str());
    }
  catch (...)
    {
      XMLPlatformUtils::Terminate();
      throw runtime_error(
          "Unknown exception importing node in XmlCTWriter::addCTData(XmlCTData*)");
    }

  try {
    rootElem->appendChild(newNode);
  }
  catch (const DOMException& e)
      {
        stringstream msg;
        msg << "Error appending node.  DOM Exception code: " << e.code << endl;
        throw runtime_error(msg.str().c_str());
      }
    catch (...)
      {
        XMLPlatformUtils::Terminate();
        throw runtime_error(
            "Unknown exception appending node in XmlCTWriter::addCTData(XmlCTData*)");
      }
}


void
XmlCTWriter::resetDOM()
{
  //! Resets the DOM, deleting all
  // contact information

  DOMNodeList* contacts = rootElem->getChildNodes();

  for(size_t i=0; i < contacts->getLength(); ++i) {
    rootElem->removeChild(contacts->item(i));
  }

}

void
XmlCTWriter::writeToFile(const string filename)
{
  // Writes out to a file

  DOMLSSerializer* ctWriter =
      ((DOMImplementationLS*) impl)->createLSSerializer();
  XMLFormatTarget* formatTarget = new LocalFileFormatTarget(filename.c_str());
  DOMLSOutput* theOutput = ((DOMImplementationLS*) impl)->createLSOutput();
  theOutput->setByteStream(formatTarget);

  if (ctWriter->getDomConfig()->canSetParameter(
      XMLUni::fgDOMWRTFormatPrettyPrint, true))
    {
      ctWriter->getDomConfig()->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint,
          true);
    }

  try {
    ctDoc->normalizeDocument();
  }
  catch (const DOMException& e)
      {
        cerr << XMLString::transcode(e.getMessage()) << endl;
        XMLPlatformUtils::Terminate();
        throw runtime_error("DOMException in XMLCTWriter::writeToFile(char*) whilst normalising document");
      }
  catch (...)
      {
        XMLPlatformUtils::Terminate();
        throw runtime_error(
            "Unknown exception in XMLCTWriter::writeToFile(char*) whilst normalising document");
      }
  try
    {
      ctWriter->write(ctDoc, theOutput);
    }
  catch (const XMLException& e)
    {
      cerr << XMLString::transcode(e.getMessage()) << endl;
      XMLPlatformUtils::Terminate();
      throw runtime_error("XMLException in XMLCTWriter::writeToFile(char*)");
    }
  catch (const DOMException& e)
    {
      cerr << XMLString::transcode(e.getMessage()) << endl;
      XMLPlatformUtils::Terminate();
      throw runtime_error("DOMException in XMLCTWriter::writeToFile(char*)");
    }
  catch (...)
    {
      XMLPlatformUtils::Terminate();
      throw runtime_error(
          "Unknown exception in XMLCTWriter::writeToFile(char*)");
    }

  theOutput->release();
  ctWriter->release();
  delete formatTarget;

}

void
XmlCTWriter::readFromFile(const string filename, map<int, XmlCTData*> ctData)
{
  // Reads existing contact.xml in


//  XercesDOMParser* parser = new XercesDOMParser();
//  parser->setValidationScheme(XercesDOMParser::Val_Never);
//  parser->setDoNamespaces(false); // optional
//
//  ErrorHandler* errHandler = (ErrorHandler*) new HandlerBase();
//  parser->setErrorHandler(errHandler);
//
//  parser->setCreateEntityReferenceNodes(true);
//  parser->setIncludeIgnorableWhitespace(false);

  DOMDocument* doc;
  XMLCh tempStr[100];
  XMLString::transcode("LS", tempStr, 99);
  DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(tempStr);
  DOMLSParser* parser = ((DOMImplementationLS*)impl)->createLSParser(DOMImplementationLS::MODE_SYNCHRONOUS, 0);

  // optionally you can set some features on this builder
  if (parser->getDomConfig()->canSetParameter(XMLUni::fgDOMValidate, false))
      parser->getDomConfig()->setParameter(XMLUni::fgDOMValidate, false);
  if (parser->getDomConfig()->canSetParameter(XMLUni::fgDOMNamespaces, false))
      parser->getDomConfig()->setParameter(XMLUni::fgDOMNamespaces, false);
  if (parser->getDomConfig()->canSetParameter(XMLUni::fgDOMElementContentWhitespace, true))
      parser->getDomConfig()->setParameter(XMLUni::fgDOMElementContentWhitespace , true);


  try
    {
      doc = parser->parseURI(filename.c_str());
    }
  catch (const XMLException& toCatch)
    {
      char* message = XMLString::transcode(toCatch.getMessage());
      cerr << "XMLException message is: \n" << message << "\n";
      XMLString::release(&message);
      throw runtime_error("XMLException");
    }
  catch (const DOMException& toCatch)
    {
      char* message = XMLString::transcode(toCatch.msg);
      cerr << "DOMException message is: \n" << message << "\n";
      XMLString::release(&message);
      throw runtime_error("DOMException");
    }
  catch (const SAXException& toCatch)
    {
      char* message = XMLString::transcode(toCatch.getMessage());
      cerr << "SAXException message is: \n" << message << "\n";
      XMLString::release(&message);
      throw runtime_error("SAX Exception");
    }
  catch (...) {
    throw runtime_error("Unknown Exception");
  }



  // Get all contact nodes, and iterate through them.
  XMLCh* contactTag = XMLString::transcode("contact");
  XMLCh* idAttr = XMLString::transcode("id");
  XMLCh* startTimeAttr = XMLString::transcode("start");
  char* cCharBuff;
  int label;
  double ctStartTime;
  XmlCTData* indiv;

  DOMNodeList* contactElems = doc->getElementsByTagName(contactTag);
  DOMElement* currElem = NULL;
  for (size_t i = 0; i < contactElems->getLength(); ++i)
    {
      currElem = (DOMElement*) contactElems->item(i);

      // Get id
      cCharBuff = XMLString::transcode(currElem->getAttribute(idAttr));
      label = atoi(cCharBuff);
      XMLString::release(&cCharBuff);

      // Get start time
      cCharBuff = XMLString::transcode(currElem->getAttribute(startTimeAttr));
      ctStartTime = atof(cCharBuff);
      XMLString::release(&cCharBuff);

      // Add contacts to map
      try
        {
          map<int, XmlCTData*>::iterator found = ctData.find(label);
          if (found == ctData.end())
            throw logic_error("Label not found in contact data");
          indiv = found->second;
        }
      catch (range_error& e)
        {
          throw EpiRisk::parse_exception(e.what());
        }

      indiv->setCTStartTime(ctStartTime);
      indiv->setID(label);

      try {
        indiv->ctData->release();
        indiv->ctData = (DOMElement*) ctDoc->importNode(currElem,true);
      }
      catch (DOMException& e) {
        char* msg = XMLString::transcode(e.getMessage());
        cerr << "Exception importing input contact to contact tracing: " << msg << endl;
        XMLString::release(&msg);
        throw EpiRisk::parse_exception("DOMException parsing contact data");
      }
      catch (...) {
        throw EpiRisk::parse_exception("Unexpected error reading contact tracing data");
      }

    }

  // Cleanup
  XMLString::release(&contactTag);
  XMLString::release(&idAttr);
  XMLString::release(&startTimeAttr);

  parser->release();


}

// XML Helper classes

XStr::XStr(const char* const toTranscode)
{
  // Call the private transcoding method
  fUnicodeForm = XMLString::transcode(toTranscode);
}

XStr::~XStr()
{
  XMLString::release(&fUnicodeForm);
}

const XMLCh*
XStr::unicodeForm() const
{
  return fUnicodeForm;
}

XNum::XNum(const size_t& myInteger)
{
  sprintf(buffer, "%lu", myInteger);

  fUnicodeForm = XMLString::transcode(buffer);
}

XNum::XNum(const double& myDouble)
{
  sprintf(buffer, "%.9lf", myDouble);

  fUnicodeForm = XMLString::transcode(buffer);
}

XNum::~XNum()
{
  XMLString::release(&fUnicodeForm);
}

const XMLCh*
XNum::unicodeForm() const
{
  return fUnicodeForm;
}
