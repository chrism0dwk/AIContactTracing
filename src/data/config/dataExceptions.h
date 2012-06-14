/* ./src/data/config/dataExceptions.h
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

#ifndef __CONFIG_EXCEPTIONS
#define __CONFIG_EXCEPTIONS

#include <exception>
#include <string>

using namespace std;

class ConfigException : public exception
{
  /** General exception for inconsistencies in the config data **/
  public:
    ConfigException();
    ConfigException(const char* msg);
    virtual const char* what() const throw();
    virtual ~ConfigException() throw();
    
  protected:
    string msg;
};



class ConfigParserException : public ConfigException
{
  /** Raised in response to a logic error in the file parser **/
  public:
    ConfigParserException(const char* msg);
};



class ConfigXmlException : public  ConfigException
{
  /** Raised in response to an XML error **/
  public:
    ConfigXmlException(const char* msg);
};



class ConfigUnknownException : public ConfigException
{
  /** Raised in response to an unknown error **/
  public:
    ConfigUnknownException(const char* msg);
};


#endif

