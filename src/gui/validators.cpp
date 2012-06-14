/* ./src/gui/validators.cpp
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

#include <wx/string.h>
#include <wx/log.h>

#include <iostream>

#include "validators.h"

//IMPLEMENT_DYNAMIC_CLASS(GenValidator,wxTextValidator)
    
BEGIN_EVENT_TABLE(GenValidator, wxTextValidator)
    EVT_CHAR(GenValidator::OnChar)
END_EVENT_TABLE()

//wxTextValidator(style,val),

GenValidator::GenValidator(long style, wxString* val) : wxTextValidator(style), myType(WXSTRING)
{
  m_validatorStyle = style;
  m_stringValue = val;
}



GenValidator::GenValidator(long style, double* val) : wxTextValidator(style), myType(DOUBLE)
{
  m_validatorStyle = style;
  m_stringValue = val;
}



GenValidator::GenValidator(long style, int* val) : wxTextValidator(style), myType(INT)
{
  m_validatorStyle = style;
  m_stringValue = val;
}


GenValidator::GenValidator(long style, string* val) : wxTextValidator(style), myType(CPPSTRING)
{
  m_validatorStyle = style;
  m_stringValue = val;
}



GenValidator::GenValidator(const GenValidator& val)
{
  Copy(val);
}



bool GenValidator::Copy(const GenValidator& val)
{
  wxValidator::Copy(val);
  
  myType = val.myType;
  m_validatorStyle = val.m_validatorStyle;
  m_stringValue = val.m_stringValue;

  m_includes    = val.m_includes;
  m_excludes    = val.m_excludes;

  return true;
}



bool GenValidator::TransferToWindow(void)
{
  if ( !CheckValidator() )
    return false;
  
  if ( m_stringValue )
  {
    wxTextCtrl* control = (wxTextCtrl*) m_validatorWindow;
    
    switch(myType) {
      case WXSTRING:
        control->ChangeValue(*(wxString*)m_stringValue);
        break;
      case DOUBLE:
        control->ChangeValue(wxString::Format(wxT("%f"),*(double*)m_stringValue));
	break;
      case INT:	
        control->ChangeValue(wxString::Format(wxT("%i"),*(int*)m_stringValue));
	break;
      case CPPSTRING:
	control->ChangeValue(wxString::FromAscii(((string*)m_stringValue)->c_str()));
	break;
    }
  }
  
  return true;
}



bool GenValidator::TransferFromWindow(void)
{
  if ( !CheckValidator() )
    return false;
  
  if ( m_stringValue )
  {
    wxTextCtrl* control = (wxTextCtrl*) m_validatorWindow;
    
    switch(myType) {
      case WXSTRING:
        *(wxString*)m_stringValue = control->GetValue();
	break;
      case DOUBLE:
	(control->GetValue()).ToDouble((double*)m_stringValue);
	break;
      case INT:
        (control->GetValue()).ToLong((long*)m_stringValue);
	break;
      case CPPSTRING:
	((string*)m_stringValue)->assign(control->GetValue().char_str());
	break;
    }
  }
  return true;
}
