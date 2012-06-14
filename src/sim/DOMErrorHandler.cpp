/* ./src/sim/DOMErrorHandler.cpp
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
 * DOMErrorHandler.cpp
 *
 *  Created on: 5 Jan 2010
 *      Author: stsiab
 */

#include <iostream>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOMError.hpp>

#include "DOMErrorHandler.hpp"

EpiDOMErrorHandler::EpiDOMErrorHandler()
{
  // TODO Auto-generated constructor stub

}

EpiDOMErrorHandler::~EpiDOMErrorHandler()
{
  // TODO Auto-generated destructor stub
}


bool EpiDOMErrorHandler::handleError(const DOMError& domError)
{
  // Display whatever error message passed from the serializer
  if (domError.getSeverity() == DOMError::DOM_SEVERITY_WARNING)
      XERCES_STD_QUALIFIER cerr << "\nWarning Message: ";
  else if (domError.getSeverity() == DOMError::DOM_SEVERITY_ERROR)
      XERCES_STD_QUALIFIER cerr << "\nError Message: ";
  else
      XERCES_STD_QUALIFIER cerr << "\nFatal Message: ";

  char *msg = XMLString::transcode(domError.getMessage());
  XERCES_STD_QUALIFIER cerr<< msg <<XERCES_STD_QUALIFIER endl;
  XMLString::release(&msg);

  // Instructs the serializer to continue serialization if possible.
  return true;

}
