/* ./src/data/config/dataUnitTest.cpp
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
 *   Copyright (C) 2008 by Chris Jewell   *
 *   c.jewell@lancaster.ac.uk   *
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


#include <iostream>

#include "config.h"

using namespace std;


int main() {
  
  string myFilename("/home/stsiab/Projects/epiRisk/trunk/src/logic/data/testConfig.xml");
  
  EpiMCMCConfig* myConfig = new EpiMCMCConfig(myFilename);
  
  set<string>* myParms;
  
  myParms = myConfig->getParameterNames();
  
  set<string>::iterator itParms = myParms->begin();
  while(itParms != myParms->end()) {
    cout << *itParms << endl;
    itParms++;
  }
  
  cout << "\n ERRORS:" << endl;
  ConfigErrorList* myErrors = myConfig->getParseErrors();
  ConfigErrorList::iterator itErrors = myErrors->begin();
  while(itErrors != myErrors->end()) {
    cout << *itErrors << endl;
    itErrors++;
  }
  
  cout << "\n WARNINGS:" << endl;
  ConfigWarningList* myWarnings = myConfig->getParseWarnings();
  ConfigWarningList::iterator itWarnings = myWarnings->begin();
  while(itWarnings != myWarnings->end()) {
    cout << *itWarnings << endl;
    itWarnings++;
  }
  
  
  delete myConfig;
  return 0;

}