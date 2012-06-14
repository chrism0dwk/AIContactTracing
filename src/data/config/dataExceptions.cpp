/* ./src/data/config/dataExceptions.cpp
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
/***************************************************************************
 *   Copyright (C) 2009 by Chris Jewell                                    *
 *   chris.jewell@warwick.ac.uk                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "dataExceptions.h"

using namespace std;

ConfigException::ConfigException(const char* _msg)
{
  msg = "Config parser error occurred";
  msg += _msg;
}


ConfigException::ConfigException()
{
}



const char* ConfigException::what() const throw()
{
  return msg.c_str();
}



ConfigException::~ConfigException() throw()
{
}



ConfigParserException::ConfigParserException(const char* _msg)
{
  msg = "Config parser error occurred: ";
  msg += _msg;
}



ConfigXmlException::ConfigXmlException(const char* _msg)
{
  msg = "XML syntax error in config file: ";
  msg += _msg;
}



ConfigUnknownException::ConfigUnknownException(const char* _msg)
{
  msg = "Unknown error in config file: ";
  msg += _msg;
}