/* ./src/data/infection.cpp
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

// Defines an infection object for an epidemic

#include <limits>

#include "infection.hpp"


#define FP_TOL 0.000


//////////////////////////////////////////////////////
// Infection class
//////////////////////////////////////////////////////

infection::infection(Ilabel_t myLabel,
		     eventTime_t myI,
		     eventTime_t myN,
		     eventTime_t myR,
		     bool isKnown,
		     infecStatus_e _status) :
  label(myLabel),
  I(myI),
  N(myN),
  R(myR),
  contactStart(myN),
  known(isKnown),
  sum_beta(0),
  sum_beta_can(0),
  status(_status),
  infecByContact(0),
  isDC(false),
  niAt(-std::numeric_limits<double>::max())
{
}



// infection::infection() :
//   known(0),
//   sum_beta(0),
//   sum_beta_can(0),
//   status(SUSCEPTIBLE),
//   infecByContact(0)
// {
// }



bool infection::operator<(infection b) const
{
  return this->label < b.label;
}



bool infection::hasContacts()
{
  // Returns true if the contact list is not empty

  if (!contacts.empty()) return true;
  else return false;
}



bool infection::isSAt(const double& time)
{
  // Returns true if *this is susceptible
  // at time

  if(status == SUSCEPTIBLE || time < I) return true;
  else return false;
}



bool infection::isIAt(const double& time)
{
  // Returns true if *this is in the Infection
  // class at time 

  if(status != SUSCEPTIBLE && I <= time && time < N) return true;
  else return false;
}



bool infection::isNAt(const double& time)
{
  // Returns true is *this is Notified
  // at time 

  if(status != SUSCEPTIBLE && N <= time && time < R) return true;
  else return false;
}



bool infection::isRAt(const double& time)
{
  // Returns true if *this is Removed
  // at time 

  if(status != SUSCEPTIBLE && R <= time) return true;
  else return false;
}



bool infection::infecInCTWindow()
{
  // Returns true if infection occurred during
  // contact tracing window
  // NB: If I>contactStart, it is still implicit that I<N

  if(I > contactStart)
    {
      //      cout << "In Contact Window!" << endl;
      //cout << "ContactStart: " << contactStart << ", I: " << I << endl;
      return true;
    }
  else
    return false;
}



bool infection::inCTWindowAt(const double& time)
{
  // Returns true if time is within our CT Window

  if(contactStart < time && time <= N) return true;
  else return false;
}



size_t infection::numNonInfecBy(CON_e contactType)
{
  // Returns the number of contacts of type contactType
  // that could potentially be infectious (ie originating
  // from a individual in I) before the infection time.

  int numContact(0);
  set<Contact>::iterator cIter = contacts.begin();

  while(cIter != contacts.end()) {

    if((cIter->time - I) <= FP_TOL) break; // Don't go beyond I

    if(cIter->type == contactType && cIter->isInfectious()) numContact++;

    cIter++;
  }
  return numContact;
}



bool infection::hasInfecContacts()
{
  // Returns true if one or more potentially
  // infectious contacts exist in the list

  set<Contact>::iterator cIter( contacts.begin() );
  
  if(!hasContacts()) return false;
  
  while(cIter != contacts.end()) {
    if(cIter->isInfectious()) return true;
    cIter++;
  }
  
  return false;
}



vector<const Contact*> infection::getInfecContacts()
{
  // Gets all infectious contacts
  
  vector<const Contact*> contactList;
  set<Contact>::iterator cIter( contacts.begin() );

  if(!hasContacts()) return contactList;

  while(cIter != contacts.end()) {
    if(cIter->isInfectious()) {
      contactList.push_back(&(*cIter));
    }
    cIter++;
  }

  return contactList;
}



size_t infection::numContactsByUntil(const CON_e contactType,
				     const double t)
{
  // Returns the number of contacts up to just before t

  size_t numContact = 0;
  set<Contact>::iterator cIter = contacts.begin();

  if(!hasContacts()) return 0;

  while(cIter != contacts.end() && (t - cIter->time) > FP_TOL) {
    if(cIter->type == contactType && cIter->isInfectious()) {
      numContact++;
    }
    cIter++;
  }

  return numContact;
}



vector<const Contact*> infection::getContactsByUntil(const CON_e contactType,
						 const double t)
{
  // Returns a vector of pointers to individuals who
  // contacted *this by method contactType up to just 
  // before time t.

  vector<const Contact*> contactList;
  set<Contact>::iterator cIter = contacts.begin();

  // Return empty vector if no contacts exist (time saver)
  if(!hasContacts()) return contactList;

  // Iterate over contacts and pick out the relevant entries
  while(cIter != contacts.end() && (t - cIter->time) > FP_TOL) {
    if(cIter->type == contactType && cIter->isInfectious()) {
      contactList.push_back(&(*cIter));
    }
    cIter++;
  }

  return contactList;
}



bool infection::isInfecContactAt(const double& t)
{
  // Returns true if an infectious contact exists at time t
  
  set<Contact>::iterator cIter( contacts.begin() );

  while( cIter != contacts.end() ) {
    if(cIter->time - t == 0 &&
       cIter->isInfectious()) return true;
    else cIter++;
  }

  return false;
}



bool infection::isInfecByWhoAt(const CON_e contactType,
			       const double& t,
			       infection*& myInfection)
{
  // Returns true if the infection time coincides
  // with a contact time of type contactType

  set<Contact>::iterator cIter( contacts.begin() );
  bool rv( false );

  while( cIter != contacts.end() ) {
    if(fabs(cIter->time - t) <= FP_TOL &&
       cIter->type == contactType && 
       cIter->isInfectious()) {
      myInfection = cIter->source;

      rv=true;
      break;
    }
    cIter++;
  }

  return rv;
}



bool infection::isInfecByWho(const CON_e contactType, const infection* contactSource)
{
  // Returns true if self->I is equal to an infectious contact
  // of type contactType, storing the source contact
  // in contactSource

  set<Contact>::iterator cIter = contacts.begin();

  while(cIter != contacts.end()) {
    if(cIter->time == I) {
      if(cIter->type == contactType && cIter->isInfectious()) {
	contactSource = cIter->source;
	return 1;
      }
      else return 0;
    }
    cIter++;
  }

  return 0;
}







void infection::updateInfecByContact()
{
  // Sets bool infection::infecByContact to true
  // if I coincides with an infectious contact

  set<Contact>::iterator cIter = contacts.begin();

  while(cIter != contacts.end()) {
    if(cIter->time == I && cIter->isInfectious()) {
      infecByContact = true;
      return;
    }
    cIter++;
  }

  infecByContact = false;

}


bool infection::isInfecByContact()
{
  // Returns true if the infection time coincides
  // with an infectious contact

  set<Contact>::iterator cIter = contacts.begin();

  while(cIter != contacts.end()) {
    if(cIter->time == I && cIter->isInfectious()) return 1;
    cIter++;
  }

  return 0;
}



bool infection::isInfecByContact(infection*& contactSource)
{
  // Returns true if self->I is equal to an infectious contact
  // of any type, stores a pointer to the source in contactSource
  
  set<Contact>::iterator cIter = contacts.begin();

  while(cIter != contacts.end()) {
    if(cIter->time == I && cIter->isInfectious()) {
      contactSource = cIter->source;
      return true;
    }
    else if(cIter->time == I && !cIter->isInfectious()) {
      cout << "WARNING in " << __PRETTY_FUNCTION__ << ": non-infec contact at infection time" << endl;
    }
    cIter++;
  }

  return false;
}



bool infection::isContactAt(const double conTime)
{
  // Returns true if a contact exists at conTime

  set<Contact>::iterator cIter = contacts.begin();

  while(cIter != contacts.end()) {
    if(cIter->time == conTime) return 1;
  }

  return 0;
}



bool infection::isContactAt(const double conTime, CON_e& type)
{
  // Returns true is a contact exists at conTime
  // AND puts the contact type into CON_e& type

  set<Contact>::iterator cIter = contacts.begin();

  while(cIter != contacts.end()) {
    if(cIter->time == conTime) {
      type = cIter->type;
      return 1;
    }
  }

  return 0;
}



void infection::attachContacts(set<Contact>& myContacts,double& myContactStart)
{
  // Attaches a list of contacts to the infection
  // and then sorts it.

  contactStart = myContactStart;
  contacts = myContacts;
}


void infection::switchInfecMethod()
{
  // Switches bool infecByContact

  if(infecByContact == false)
    infecByContact = true;
  else
    infecByContact = true;
}



///////////////////////////////////////////////////////////////////////////////
// Contact class
///////////////////////////////////////////////////////////////////////////////


Contact::Contact(infection* conSource, const CON_e conType, const eventTime_t conTime) :
  source(conSource),
  type(conType),
  time(conTime)
{
}



Contact::Contact(const Contact& c) :
  source(c.source),
  type(c.type),
  time(c.time)
{
  // Copy constructor
}



bool Contact::isInfectious() const
{
  // Returns true if the contact
  // is infectious at the time of the contact

  if(source->isIAt(time)) return true;
  else return false;
}



Contact Contact::operator=(const Contact& c)
{
  source = c.source;
  const_cast<CON_e&>(type) = c.type;
  const_cast<eventTime_t&>(time) = c.time;
  return *this;
}



bool Contact::operator<(const Contact& rhs) const
{
  return time < rhs.time;
}






bool Contact::operator==(const Contact& rhs) const
{
  return time == rhs.time;
}
