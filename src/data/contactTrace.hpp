/* ./src/data/contactTrace.hpp
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

// Classes related to contact tracing

#ifndef INCLUDE_CONTACTTRACE_HPP
#define INCLUDE_CONTACTTRACE_HPP

#include <list>
#include <stdexcept>

#include "aiTypes.hpp"

using namespace std;


// State enum and function
enum CON_e {
  FEEDMILL,
  SHOUSE,
  COMPANY
};




class Contact {
 public:
  const size_t source;
  const CON_e type;
  const eventTime_t time;
  bool infectious;

  Contact(const int conSource, const CON_e conType, const eventTime_t conTime, const bool conInfec = 0);
  Contact(const Contact&);
  Contact operator=(const Contact&);
  bool operator<(const Contact&);

};

#endif
