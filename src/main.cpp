/* ./src/main.cpp
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


#include <wx/wx.h>

#include "gui-main.h"


class epiRiskapp : public wxApp
{
  public:
    virtual bool OnInit();
};


IMPLEMENT_APP(epiRiskapp);



bool epiRiskapp::OnInit()
{
  epiRiskFrame *frame = new epiRiskFrame( wxT( "epiRisk Config Editor" ), wxDefaultPosition, wxSize(500,460) );

  frame->Show(TRUE);
  SetTopWindow(frame);
  return TRUE;
}



