/* ./src/sim/DOMErrorHandler.hpp
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
 * DOMErrorHandler.hpp
 *
 *  Created on: 5 Jan 2010
 *      Author: stsiab
 */

#ifndef DOMERRORHANDLER_HPP_
#define DOMERRORHANDLER_HPP_

#include <xercesc/dom/DOMErrorHandler.hpp>

XERCES_CPP_NAMESPACE_USE


class EpiDOMErrorHandler : public DOMErrorHandler
{
public:
  EpiDOMErrorHandler();
  virtual
  ~EpiDOMErrorHandler();

  bool handleError(const DOMError& domError);
  void resetErrors(){};

private:
  EpiDOMErrorHandler(const DOMErrorHandler&);
  void operator=(const DOMErrorHandler&);
};

#endif /* DOMERRORHANDLER_HPP_ */
