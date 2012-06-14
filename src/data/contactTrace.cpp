/* ./src/data/contactTrace.cpp
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

// XML Contacts file parser

#include "contactTrace.hpp"








Contact::Contact(const int conSource, const CON_e conType, const eventTime_t conTime, const bool conInfec) :
  source(conSource),
  type(conType),
  time(conTime),
  infectious(conInfec)
{
}



Contact::Contact(const Contact& c) :
  source(c.source),
  type(c.type),
  time(c.time),
  infectious(c.infectious)
{
  // Copy constructor
}



Contact Contact::operator=(const Contact& c)
{
  const_cast<size_t&>(source) = c.source;
  const_cast<CON_e&>(type) = c.type;
  const_cast<eventTime_t&>(time) = c.time;
  infectious = c.infectious;
  return *this;
}



bool Contact::operator<(const Contact& rhs)
{
  return time < rhs.time;
}



