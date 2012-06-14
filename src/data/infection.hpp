/* ./src/data/infection.hpp
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

#ifndef INCLUDE_INFECTION_HPP
#define INCLUDE_INFECTION_HPP

/////////////////////////////////
// FWD DECLS
/////////////////////////////////

class Contact;
class infection;



/////////////////////////////////
// ENUMs
/////////////////////////////////


enum CON_e {
  // Connection types
  FEEDMILL,
  SHOUSE,
  COMPANY
};



/////////////////////////////////
// Infection class
/////////////////////////////////



#include <set>
#include <vector>
#include <iostream>
#include <math.h>

#include "aiTypes.hpp"
//#include "contactTrace.hpp"

using namespace std;

enum infecStatus_e {
  SUSCEPTIBLE = 0,
  INFECTED
};

class infection {
public:
  Ilabel_t label;
  eventTime_t I; // Changes
  eventTime_t N;
  eventTime_t R;
  bool known;
  set<Contact> contacts;
  vector<size_t> connections;
  eventTime_t contactStart;
  double sum_beta;
  double sum_beta_can;
  infecStatus_e status;
  bool infecByContact;
  bool isDC;  // Indicates premature detection and culling
  double niAt;  // Indicates that we have "not infected" information

  bool operator<(infection) const;

  infection(Ilabel_t myLabel,
	    eventTime_t myI,
	    eventTime_t myN,
	    eventTime_t myR,
	    bool isKnown=0,
	    infecStatus_e _status=SUSCEPTIBLE); // Constructor for an infection
  //infection();

  void attachContacts(set<Contact>&,double&);
  bool hasContacts();
  bool isSAt(const double&);
  bool isIAt(const double&);
  bool isNAt(const double&);
  bool isRAt(const double&);
  bool infecInCTWindow();
  bool inCTWindowAt(const double&);
  bool hasInfecContacts();
  vector<const Contact*> getInfecContacts();
  size_t numNonInfecBy(CON_e);
  bool isInfecContactAt(const double&);
  bool isInfecByWhoAt(const CON_e, const double&, infection*&);
  bool isInfecByWho(const CON_e, const infection*);
  void updateInfecByContact();
  bool isInfecByContact();
  bool isInfecByContact(infection*&);
  bool isContactAt(const double);
  bool isContactAt(const double,CON_e&);
  size_t numContactsByUntil(const CON_e, const double);
  vector<const Contact*> getContactsByUntil(const CON_e, const double);
  void switchInfecMethod();

};






///////////////////////////////////////////////////
// Classes related to contact tracing
///////////////////////////////////////////////////

#include <list>
#include <stdexcept>

#include "aiTypes.hpp"

using namespace std;






class Contact {
 public:
  infection* source;
  const CON_e type;
  const eventTime_t time;
  
  Contact(infection* conSource, const CON_e conType, const eventTime_t conTime);
  Contact(const Contact&);
  Contact operator=(const Contact&);
  bool operator<(const Contact&) const;
  bool operator==(const Contact&) const;
  bool isInfectious() const;
};

#endif
