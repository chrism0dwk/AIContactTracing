/* ./src/gui/validators.h
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


#ifndef _GUI_VALIDATORS_H_
#define _GUI_VALIDATORS_H_

#if wxUSE_VALIDATORS && wxUSE_TEXTCTRL

#include <wx/textctrl.h>
#include <wx/valtext.h>

#include <string>

using namespace std;


class GenValidator : public wxTextValidator
  {
//DECLARE_DYNAMIC_CLASS(GenValidator)
  public:
    GenValidator(long style = wxFILTER_NONE, wxString* val = 0);
    GenValidator(long style = wxFILTER_NONE, double* val = 0);
    GenValidator(long style = wxFILTER_NONE, int* val = 0);
    GenValidator(long style = wxFILTER_NONE, string* val=0);
    GenValidator(const GenValidator& val);
    
    bool Copy(const GenValidator& val);
    virtual wxObject* Clone() const {return new GenValidator(*this);}

    virtual ~GenValidator(){};

    virtual bool TransferToWindow();
    virtual bool TransferFromWindow();

  private:
    GenValidator& operator=(const GenValidator&);
    
    enum GENVALTYPE
    {
      WXSTRING = 0,
      DOUBLE,
      INT,
      CPPSTRING
    };

    GENVALTYPE myType;
    void* m_stringValue;
    char cStringBuff[100];
    
    DECLARE_EVENT_TABLE()
  };

#endif


#endif
