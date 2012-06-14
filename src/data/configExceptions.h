/* ./src/data/configExceptions.h
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

#ifndef RESANRCONFIGEXCEPTIONS_H
#define RESANRCONFIGEXCEPTIONS_H

#include <exception>

namespace resanr {

/** Provides an exception class for configuration file errors.
	@author Chris Jewell <chris.jewell@warwick.ac.uk>
*/
class config_error : public std::exception
{
public:
    config_error(const char* msg);
    const char* what() const throw();
private:
    const char* msg;

};

}

#endif
