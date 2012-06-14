/* ./src/gui/gui-main.h
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



#ifndef _EPIRISK_H_
#define _EPIRISK_H_



#include <wx/filename.h>
#include <wx/arrstr.h>
#include <wx/filepicker.h>
#include <wx/grid.h>

#include <string>

#include "epiMCMCConfig.h"

/**
 * @short epiRisk Main Window
 * @author Chris Jewell <chris.jewell@warwick.ac.uk>
 * @version 0.1
 */

// Constants

#define INIT_PRIORCHOICES wxArrayString PRIORCHOICES; \
                                        PRIORCHOICES.Add(wxT("Beta")); \
                                        PRIORCHOICES.Add(wxT("Gamma"));







class ConstantList : public wxListCtrl
  {
  public:
    ConstantList(wxWindow* parent, wxWindowID id, long style);
  };



class ParmPanel : public wxPanel
  {
  public:
    ParmPanel(wxWindow* parent);
    void importData(wxInitDialogEvent& event);
    void OnSelectParm(wxListEvent& event);
    void OnDeselectParm(wxListEvent& event);
    void OnChangeStartVal(wxCommandEvent& event);
    void OnChangeTuningConst(wxCommandEvent& event);
    void OnResizePriorGrid(wxSizeEvent& event);
    void OnSelectPrior(wxListEvent& event);
    void OnChangePriorParm(wxGridEvent& event);
    void OnBeginEditPrior(wxListEvent& event);
    void OnEndEditPrior(wxListEvent& event);
    
  private:
  wxListCtrl* parmList;
  wxTextCtrl* startVal;
  wxTextCtrl* tuningConst;
  wxChoice* priorComboBox;
  wxGrid* priorList;
  ParmData* pCurrentParm;
  };



class DataPanel : public wxPanel
  {
  public:
    DataPanel(wxWindow* parent);
  };



class OptionPanel : public wxPanel
  {
  public:
    OptionPanel(wxWindow* parent);
  };



class DirField : public wxPanel
  {
  public:
    DirField(wxWindow* parent, wxWindowID id, const wxString& name, string* _fileString);
    void OnChange(wxFileDirPickerEvent& event);
  private:
    string* fileString;
  };



class FileField : public wxPanel
  {
  public:
    FileField(wxWindow* parent, wxWindowID id, const wxString& name, string* _fileString);
    void OnChange(wxFileDirPickerEvent& event);
    
  private:
    string* fileString;
  };

  


class
      epiRiskFrame : public wxFrame
  {
  public:
    
    epiRiskFrame( const wxString& title, const wxPoint& pos, const wxSize& size );
    void OnNew(wxCommandEvent& event);

    // File actions
    void OnOpen(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnSaveAs(wxCommandEvent& event);
    void OnQuit( wxCommandEvent& event );

    // Edit actions
    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);
    void OnCut(wxCommandEvent& event);
    void OnCopy(wxCommandEvent& event);
    void OnPaste(wxCommandEvent& event);

    // Help actions
    void OnAbout( wxCommandEvent& event );
    
    // Populating the data
    void populate();

  private:

    void buildMenuBar();
    void buildToolBar();
    void buildContent();
    bool save();
    bool isOkToClose();
    void initFile();
  };







#endif // _EPIRISK_H_
