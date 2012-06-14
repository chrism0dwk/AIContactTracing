/* ./src/sim/EventDom.hpp
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
 * Event.hpp
 *
 *  Created on: 5 Jan 2010
 *      Author: stsiab
 */

#ifndef EVENT_HPP_
#define EVENT_HPP_

#include <string>
#include <set>

using namespace std;


enum ContactType { BACKGROUND = 0,
                   FEEDMILL,
                   SHOUSE,
                   COMPANY,
                   ISPATIAL,
                   NSPATIAL
};



class Event
{
public:
  Event(const size_t id_, const double time_);
  virtual
  ~Event();

  const size_t id;
  const double time;

};



class ContactEvent : public Event
{
  // Represents a contact event
public:
  ContactEvent(const size_t id_,
               const double time_,
               const size_t from_,
               const ContactType type_);

  const size_t from;
  const ContactType type;
};



class ContactQueueComp
{
public:
  bool operator() (const ContactEvent* lhs, const ContactEvent* rhs) const;
};



class ContactQueue
{
  // Represents an ordered (early -> late) queue of events
  //  accessed by a next() method
public:
  ContactQueue(const string filename);
  ~ContactQueue();
  void reset();  // Resets the queue pointer to the start
  ContactEvent* next(); // Returns the next contact event, NULL if no more contact data

private:
  typedef set< ContactEvent*, ContactQueueComp > EventQueue;
  EventQueue eventQueue;
};

#endif /* EVENT_HPP_ */
