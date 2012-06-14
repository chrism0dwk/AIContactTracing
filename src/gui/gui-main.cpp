/* ./src/gui/gui-main.cpp
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

// GUI includes /////////////
#include <wx/wx.h>
#include <wx/log.h>
#include <wx/notebook.h>
#include <wx/splitter.h>
#include <wx/treectrl.h>
#include <wx/artprov.h>
#include <wx/imaglist.h>
#include <wx/filename.h>
#include <wx/filepicker.h>
#include <wx/valtext.h>
/////////////////////////////

#include <iostream>
#include <string>

#include "gui-main.h"
#include "config.h"
#include "validators.h"


// Global data model
EpiMCMCConfig config;


// IDs

enum guiID
{
  guiDATA_EPIDATA = 1001,
  guiDATA_COVARPREFIX,
  guiDATA_OUTPUTDIR,
  guiPARM_PARMLIST,
  guiPARM_STARTVAL,
  guiPARM_MHCONST,
  guiPARM_PRIORLIST
};






epiRiskFrame::epiRiskFrame( const wxString& title, const wxPoint& pos, const wxSize& size )
    : wxFrame((wxFrame *)NULL, -1, title, pos, size)
{


  initFile();

  buildMenuBar();
  buildToolBar();
  buildContent();

  CreateStatusBar();
  SetStatusText( wxT( "Welcome to epiRisk configuration editor" ) );

}



void epiRiskFrame::buildMenuBar()
{
  // Build the application menu

  wxMenuBar *menuBar = new wxMenuBar;

  // File menu
  wxMenu *menuFile = new wxMenu;
  menuFile->Append( wxID_NEW, wxT ( "&New\tCtrl+N" ) );
  menuFile->Append( wxID_OPEN, wxT( "&Open...\tCtrl+O" ) );
  menuFile->Append( wxID_CLOSE, wxT( "&Close\tCtrl+W" ) );
  menuFile->AppendSeparator();
  menuFile->Append( wxID_SAVE, wxT( "&Save\tCtrl+S" ) );
  menuFile->Append( wxID_SAVEAS, wxT( "Save &As..." ) );
  menuFile->AppendSeparator();
  menuFile->Append( wxID_EXIT, wxT( "E&xit\tCtrl+Q" ) );

  menuBar->Append( menuFile, wxT( "&File" ) );

  Connect(wxID_NEW,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(epiRiskFrame::OnNew));
  Connect(wxID_OPEN,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(epiRiskFrame::OnOpen));
  Connect(wxID_CLOSE,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(epiRiskFrame::OnClose));
  Connect(wxID_SAVE,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(epiRiskFrame::OnSave));
  Connect(wxID_SAVEAS,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(epiRiskFrame::OnSaveAs));
  Connect(wxID_EXIT,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(epiRiskFrame::OnQuit));


  // Edit menu
  wxMenu* menuEdit = new wxMenu;
  menuEdit->Append( wxID_UNDO, wxT( "&Undo\tCtrl+Z" ) );
  menuEdit->Append( wxID_REDO, wxT( "&Redo\tCtrl+Shift+Z" ) );
  menuEdit->AppendSeparator();
  menuEdit->Append( wxID_CUT, wxT( "Cu&t\tCtrl+X" ) );
  menuEdit->Append( wxID_COPY, wxT( "&Copy\tCtrl+C" ) );
  menuEdit->Append( wxID_PASTE, wxT( "&Paste\tCtrl+V" ) );

  menuBar->Append( menuEdit, wxT( "&Edit" ) );

  Connect(wxID_UNDO,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(epiRiskFrame::OnUndo));
  Connect(wxID_REDO,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(epiRiskFrame::OnRedo));
  Connect(wxID_CUT,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(epiRiskFrame::OnCut));
  Connect(wxID_COPY,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(epiRiskFrame::OnCopy));
  Connect(wxID_PASTE,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(epiRiskFrame::OnPaste));


  // Help menu
  wxMenu* menuHelp = new wxMenu;
  menuHelp->Append( wxID_ABOUT, wxT( "&About...") );

  menuBar->Append( menuHelp, wxT( "&Help" ) );

  Connect(wxID_ABOUT,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(epiRiskFrame::OnAbout));


  // Build bar...
  SetMenuBar( menuBar );
}



void epiRiskFrame::buildToolBar()
{
  // Builds the tool bar



  wxToolBar* toolBar = CreateToolBar(wxTB_DOCKABLE);
  toolBar->AddTool(wxID_NEW,wxT("New..."),wxArtProvider::GetBitmap(wxART_NORMAL_FILE,wxART_TOOLBAR));
  toolBar->AddTool(wxID_OPEN,wxT("Open..."),wxArtProvider::GetBitmap(wxART_FOLDER,wxART_TOOLBAR));
  toolBar->AddTool(wxID_SAVE,wxT("Save"),wxArtProvider::GetBitmap(wxART_FILE_SAVE,wxART_TOOLBAR));
  toolBar->AddSeparator();
  toolBar->AddTool(wxID_CUT,wxT("Cut"),wxArtProvider::GetBitmap(wxART_CUT,wxART_OTHER));
  toolBar->AddTool(wxID_COPY,wxT("Copy"),wxArtProvider::GetBitmap(wxART_COPY,wxART_OTHER));
  toolBar->AddTool(wxID_PASTE,wxT("Paste"),wxArtProvider::GetBitmap(wxART_PASTE,wxART_OTHER));
  toolBar->Realize();


}



void epiRiskFrame::buildContent()
{

  // Set top level sizer
  wxGridSizer* topLevelSizer = new wxGridSizer(1,1,0,0);


  // Set up tabs
  wxNotebook* tabs = new wxNotebook(this,wxID_ANY,wxPoint(0,0),wxSize(-1,-1),wxNB_TOP,wxT("Tabs"));
  ParmPanel* parmPanel = new ParmPanel(tabs);
  OptionPanel* optionPanel = new OptionPanel(tabs);
  DataPanel* dataPanel = new DataPanel(tabs);

  tabs->AddPage(parmPanel, wxT("Parameters"),true);
  tabs->AddPage(optionPanel, wxT("Options"),false);
  tabs->AddPage(dataPanel, wxT("Data"),false);

  topLevelSizer->Add(tabs,0,wxGROW | wxALL, 1);

  SetSizer(topLevelSizer);

}



// Actions

void epiRiskFrame::OnNew( wxCommandEvent& WXUNUSED( event ) )
{

}



void epiRiskFrame::OnOpen( wxCommandEvent& WXUNUSED( event ) )
{
  wxFileDialog* openFileDialog = new wxFileDialog(this,wxT("Open file..."),wxFileName::GetCwd(),wxT(""),wxT("epiRisk config file (*.xml)|*.xml|All files (*.*)|*.*"));

  if (openFileDialog->ShowModal() == wxID_OK)
    {
      wxString fileName = openFileDialog->GetPath();
//      cwd->Assign(fileName);
    }
}


void epiRiskFrame::OnSave( wxCommandEvent& WXUNUSED( event ) )
{
  save();

}



void epiRiskFrame::OnSaveAs( wxCommandEvent& WXUNUSED( event ))
{
  save();
}



void epiRiskFrame::OnClose( wxCommandEvent& WXUNUSED( event ) )
{

  if (!isOkToClose())
    {
      if (save())
        {
          // Close document here
        }
    }
}



void epiRiskFrame::OnQuit( wxCommandEvent& WXUNUSED( event ) )
{
  if (isOkToClose()) Close(TRUE);
  else
    {
      if (save())
        Close(TRUE);
    }

}



void epiRiskFrame::OnUndo( wxCommandEvent& WXUNUSED( event ) )
{
}



void epiRiskFrame::OnRedo( wxCommandEvent& WXUNUSED( event ) )
{
}



void epiRiskFrame::OnCut( wxCommandEvent& WXUNUSED( event ) )
{
}



void epiRiskFrame::OnCopy( wxCommandEvent& WXUNUSED( event ) )
{
}



void epiRiskFrame::OnPaste( wxCommandEvent& WXUNUSED( event ) )
{
}



void epiRiskFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
  wxMessageBox( wxT( "epiRisk config editor helps you edit configuration files for aiMCMC.\n\n(c) C. Jewell 2008" ),
                wxT( "About epiRisk config editor" ), wxOK | wxICON_INFORMATION, this );
}



bool epiRiskFrame::isOkToClose()
{

//   if (fileState == FILE_MODIFIED || fileState == FILE_NEW)
//     {
//       wxMessageDialog* saveQuestion = new wxMessageDialog(NULL,wxT("File modified, do you want to save it?"),wxT("Warning"),wxYES_NO | wxYES_DEFAULT | wxICON_QUESTION);
//
//       if (saveQuestion->ShowModal() == wxID_YES)
//         {
//           return false;
//         }
//       else
//         {
//           return true;
//         }
//     }
//   else return true;
  return true;

}



bool epiRiskFrame::save()
{
  // Saves the file
  wxFileDialog* openFileDialog = new wxFileDialog(this,wxT("Save As..."),wxFileName::GetCwd(),wxT(""),wxT("epiRisk config file (*.xml)|*.xml|All files (*.*)|*.*"),wxFD_SAVE);

  if (openFileDialog->ShowModal() == wxID_OK)
    {
      // Save the file - replace with Exception handling system!
      if (1==1)
        {
          wxMessageDialog* msg = new wxMessageDialog(NULL,openFileDialog->GetPath(),wxT("Saved"), wxOK | wxICON_INFORMATION);
          msg->ShowModal();
          return true;
        }
      else
        {
          wxMessageDialog* msg = new wxMessageDialog(NULL,wxT("Could not save file!"),wxT("Error"), wxOK | wxICON_WARNING);
          msg->ShowModal();
          return false;
        }
    }
  else return false;

}


void epiRiskFrame::initFile()
{
  //
  // Read in blank config file here
  //

}



ConstantList::ConstantList(wxWindow* parent, wxWindowID id,long style)
    : wxListCtrl(parent,id,wxDefaultPosition,wxDefaultSize,style)
{
  InsertColumn(0,wxT("Parameter"));
  InsertColumn(1,wxT("Value"));
}



ParmPanel::ParmPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
  // Set up parmPanel

  wxStaticText* staticText;

  wxSplitterWindow* splitter = new wxSplitterWindow(this,wxID_ANY,wxDefaultPosition,wxSize(400,-1),wxSP_3D | wxSP_BORDER | wxSP_LIVE_UPDATE );

  parmList = new wxListCtrl(splitter, guiPARM_PARMLIST,wxDefaultPosition,wxSize(-1,-1),wxLC_LIST | wxLC_NO_HEADER | wxLC_SORT_ASCENDING | wxLC_SINGLE_SEL | wxBORDER_SUNKEN);
  wxPanel* rightSide = new wxPanel(splitter,wxID_ANY);

  splitter->SetMinimumPaneSize(100);
  splitter->SplitVertically(parmList,rightSide,100);

  wxBoxSizer* parmGridSizer = new wxBoxSizer(wxHORIZONTAL);
  parmGridSizer->Add(splitter,1,wxGROW | wxALL,5);
  this->SetSizer(parmGridSizer);

  // Set up parameter edit widgets
  wxBoxSizer* editBoxVSizer = new wxBoxSizer(wxVERTICAL);
  rightSide->SetSizer(editBoxVSizer);

  // Start value
  wxBoxSizer* startValHSizer = new wxBoxSizer(wxHORIZONTAL);
  staticText = new wxStaticText(rightSide,wxID_ANY,wxT("&Start value:"));
  startVal = new wxTextCtrl(rightSide,guiPARM_STARTVAL,wxT(""),wxDefaultPosition,wxDefaultSize,wxTE_READONLY);
  startValHSizer->Add(staticText,0,wxALIGN_CENTER_VERTICAL);
  startValHSizer->Add(startVal,1,wxEXPAND | wxALIGN_CENTER_VERTICAL | wxLEFT,5);
  editBoxVSizer->Add(startValHSizer,0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT,5);

  // Tuning constants
  wxBoxSizer* tcHSizer = new wxBoxSizer(wxHORIZONTAL);
  staticText = new wxStaticText(rightSide,wxID_ANY,wxT("&Tuning constant:"));
  tuningConst = new wxTextCtrl(rightSide,guiPARM_MHCONST,wxT(""),wxDefaultPosition,wxDefaultSize,wxTE_READONLY);
  tcHSizer->Add(staticText,0,wxALIGN_CENTER_VERTICAL);
  tcHSizer->Add(tuningConst,1,wxEXPAND | wxALIGN_CENTER_VERTICAL | wxLEFT,5);
  editBoxVSizer->Add(tcHSizer,0,wxEXPAND | wxTOP | wxLEFT | wxRIGHT,5);


  // Priors

  // Set up choices:

  INIT_PRIORCHOICES;

  wxStaticBoxSizer* priorBoxSizer = new wxStaticBoxSizer(wxVERTICAL,rightSide,wxT("Prior"));

  wxBoxSizer* distroHSizer = new wxBoxSizer(wxHORIZONTAL);
  staticText = new wxStaticText(rightSide,wxID_ANY,wxT("Distribution:"));

  priorComboBox = new wxChoice(rightSide,wxID_ANY,wxDefaultPosition,wxDefaultSize,PRIORCHOICES);
  distroHSizer->Add(staticText,0,wxALIGN_CENTER_VERTICAL);
  distroHSizer->Add(priorComboBox,1,wxEXPAND | wxALIGN_CENTER_VERTICAL | wxLEFT,5);
  priorBoxSizer->Add(distroHSizer,0,wxEXPAND | wxALIGN_LEFT, 0);

//   priorList = new ConstantList(rightSide,guiPARM_PRIORLIST,wxLC_EDIT_LABELS | wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxBORDER_SUNKEN );
  priorList = new wxGrid(rightSide,guiPARM_PRIORLIST,wxDefaultPosition,wxDefaultSize, wxBORDER_SUNKEN);

  wxGridCellAttr* nameColAttr = new wxGridCellAttr;
  nameColAttr->SetReadOnly();
  priorList->CreateGrid(0,2);
  priorList->SetRowLabelSize(0);
  priorList->SetColLabelSize(20);
  priorList->SetColLabelValue(0,wxString(wxT("Parameter")));
  priorList->SetColLabelValue(1,wxString(wxT("Value")));
  priorList->SetColAttr(0,nameColAttr);
  wxLogDebug(wxT("Prior Box Size: %i"),priorList->GetSize().GetWidth());
  priorBoxSizer->Add(priorList,1,wxEXPAND | wxALL,10);

  editBoxVSizer->Add(priorBoxSizer,1,wxEXPAND | wxALL,5);


  // Connect events with slots

  Connect(wxEVT_INIT_DIALOG,wxInitDialogEventHandler(ParmPanel::importData));
  Connect(guiPARM_PARMLIST,wxEVT_COMMAND_LIST_ITEM_SELECTED,wxListEventHandler(ParmPanel::OnSelectParm));
  Connect(guiPARM_PARMLIST,wxEVT_COMMAND_LIST_ITEM_DESELECTED,wxListEventHandler(ParmPanel::OnDeselectParm));

  startVal->Connect(wxEVT_COMMAND_TEXT_UPDATED,wxCommandEventHandler(ParmPanel::OnChangeStartVal),NULL,this);

  tuningConst->Connect(wxEVT_COMMAND_TEXT_UPDATED,wxCommandEventHandler(ParmPanel::OnChangeTuningConst),NULL,this);

  priorList->Connect(wxEVT_SIZE,wxSizeEventHandler(ParmPanel::OnResizePriorGrid),NULL,this);
//   priorList->Connect(wxEVT_COMMAND_LIST_ITEM_ACTIVATED,wxListEventHandler(ParmPanel::OnSelectPrior));
//   priorList->Connect(wxEVT_COMMAND_LIST_BEGIN_LABEL_EDIT,wxListEventHandler(ParmPanel::OnBeginEditPrior));
//   priorList->Connect(wxEVT_COMMAND_LIST_END_LABEL_EDIT,wxListEventHandler(ParmPanel::OnEndEditPrior));
  priorList->Connect(wxEVT_GRID_CELL_CHANGE,wxGridEventHandler(ParmPanel::OnChangePriorParm),NULL,this);


  InitDialog();
}



void ParmPanel::importData(wxInitDialogEvent& WXUNUSED(event))
{
  // Imports data from an EpiMCMCConfig element.
  map<string,ParmData>::iterator it = config.parmData.begin();
  int index = 0;

  while (it != config.parmData.end() )
    {
      wxString item = wxString::FromAscii(it->first.c_str());
      parmList->InsertItem(index, item);
      parmList->SetItemPtrData(index,(wxUIntPtr)&(it->second));
      index++;
      it++;
    }


}



void ParmPanel::OnSelectParm(wxListEvent& event)
{

  wxLogDebug(wxT("OnSelectParm"));

  if (event.GetData() == NULL)
    {
      pCurrentParm = NULL;
      wxLogDebug(wxT("Null data in OnSelectParm"));
      return;
    }

  double val = 0.0;

  pCurrentParm = (ParmData*)event.GetData();

  // Start value
  val = pCurrentParm->getStartVal();
  wxLogDebug(wxT("In OnSelectParm, (%x) startVal: %f"),pCurrentParm,val);
  startVal->ChangeValue(wxString::Format(wxT("%f"),val));
  startVal->SetEditable(true);

  // Get tuning constant
  val = pCurrentParm->getMHTuning();
  tuningConst->ChangeValue(wxString::Format(wxT("%f"),val));
  tuningConst->SetEditable(true);

  // Priors
  const map<string,double>* hyperParms = pCurrentParm->getHyperParms();
  map<string,double>::const_iterator it = hyperParms->begin();
  int index = 0;
  while (it != hyperParms->end())
    {

      /*    priorList->InsertItem(index,wxString::FromAscii(it->first.c_str()));
          priorList->SetItem(index,1,wxString::Format(wxT("%f"),it->second));*/
      priorList->AppendRows(1);
      priorList->SetCellValue(index,0,wxString::FromAscii(it->first.c_str()));
      priorList->SetCellValue(index,1,wxString::Format(wxT("%f"),it->second));

      index++;
      it++;
    }

}



void ParmPanel::OnDeselectParm(wxListEvent& event)
{
  wxLogDebug(wxT("OnDeselectParm (%x)"),pCurrentParm);
  pCurrentParm = NULL;

  // Clear the start value
  //startVal->Clear();
  startVal->ChangeValue(wxT(""));
  startVal->SetEditable(false);

  // Clear the tuning constant
  tuningConst->Clear();
  tuningConst->SetEditable(false);

  // Clear the priorList
//   priorList->DeleteAllItems();
  priorList->DeleteRows(0,priorList->GetNumberRows());

}



void ParmPanel::OnChangeStartVal(wxCommandEvent& event)
{

  wxLogDebug(wxT("OnChangeStartVal (%x)"),pCurrentParm);
  // Check if pCurrentParm is NOT null
  if (pCurrentParm == NULL)
    {
      wxLogDebug(wxT("pCurrentParm is null"));
      return;
    }

  // Update data model
  double val;
  event.GetString().ToDouble(&val);
  pCurrentParm->setStartVal(val);
  wxLogDebug(wxT("Parameter %x startVal changed to %f"),pCurrentParm, pCurrentParm->getStartVal());

}



void ParmPanel::OnChangeTuningConst(wxCommandEvent& event)
{
  // Check if pCurrentParm is NOT null
  if (pCurrentParm == 0)
    {
      wxLogDebug(wxT("pCurrentParm is null"));
      return;
    }

  // Update data model
  double val;
  event.GetString().ToDouble(&val);
  pCurrentParm->setMHTuning(val);
}



void ParmPanel::OnResizePriorGrid(wxSizeEvent& event)
{
  wxLogDebug(wxT("Resize prior grid"));

  wxSize size = event.GetSize();
  wxLogDebug(wxT("Width: %i"),size.GetWidth());
  priorList->SetDefaultColSize((size.GetWidth()-10)/2,true);
}


void ParmPanel::OnSelectPrior(wxListEvent& event)
{
  wxListItem item = event.GetItem();
}



void ParmPanel::OnChangePriorParm(wxGridEvent& event)
{
  if (pCurrentParm == 0)
    {
      wxLogDebug(wxT("pCurrentParm is null in OnChangePriorParm"));
      return;
    }

  // Update data model
  int row = event.GetRow();
  double val;
  priorList->GetCellValue(row,1).ToDouble(&val);
  string parmName = string(priorList->GetCellValue(row,0).char_str());

  map<string,double>* priorParms = pCurrentParm->getHyperParms();

  map<string,double>::iterator itParm = priorParms->find(parmName);

  wxASSERT(itParm != priorParms->end());

  itParm->second = val;
  wxLogDebug(wxT("Hyperparm %s changed to %f"),priorList->GetCellValue(row,0).c_str(),val);
}



void ParmPanel::OnBeginEditPrior(wxListEvent& event)
{


}



void ParmPanel::OnEndEditPrior(wxListEvent& event)
{


}


DataPanel::DataPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{

  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

  // Epidemic data
  FileField* epiData = new FileField(this,guiDATA_EPIDATA,wxT("&Epidemic data:"), &config.epiDataFilename);

  // Covariate data prefix
  FileField* covars = new FileField(this,guiDATA_COVARPREFIX,wxT("&Covariate prefix:"), &config.covarPrefix);

  // Output directory
  DirField* output = new DirField(this,guiDATA_OUTPUTDIR,wxT("&Output folder:"), &config.outputFolder);

  // Output prefix

  wxBoxSizer* prefixBox = new wxBoxSizer(wxHORIZONTAL);
  wxStaticText* prefixTitle = new wxStaticText(this,wxID_ANY,wxT("Output file &prefix:"));
  wxTextCtrl* prefixTextCtrl = new wxTextCtrl(this,wxID_ANY,wxT(""),wxDefaultPosition,wxDefaultSize,0,
      GenValidator(wxFILTER_ALPHANUMERIC, &(config.outputPrefix)));

  prefixBox->Add(prefixTitle,0,wxALIGN_CENTER_VERTICAL);
  prefixBox->Add(prefixTextCtrl,1,wxALIGN_CENTER_VERTICAL,5);

  mainSizer->Add(epiData,1,wxEXPAND | wxALL,10);
  mainSizer->Add(covars,1,wxEXPAND | wxALL,10);
  mainSizer->Add(output,1,wxEXPAND | wxALL,10);
  mainSizer->Add(prefixBox,1,wxEXPAND | wxALL,10);

  SetSizer(mainSizer);

  TransferDataToWindow();
}



DirField::DirField(wxWindow* parent, wxWindowID id, const wxString& name, string* _fileString)
    : wxPanel(parent, wxID_ANY), fileString(_fileString)
{
  wxBoxSizer* epiDataSizer = new wxBoxSizer(wxHORIZONTAL);
  wxStaticText* staticText = new wxStaticText(this,wxID_ANY,name);
  wxDirPickerCtrl* epiDataEntry = new wxDirPickerCtrl(this,id,wxString::FromAscii(fileString->c_str()),wxT("Select a folder"),wxDefaultPosition,wxDefaultSize,wxDIRP_USE_TEXTCTRL | wxFLP_CHANGE_DIR);

  epiDataSizer->Add(staticText,0,wxALIGN_CENTER_VERTICAL);
  epiDataSizer->Add(epiDataEntry,1,wxALIGN_CENTER_VERTICAL | wxALL,5);

  SetSizer(epiDataSizer);

}



void DirField::OnChange( wxFileDirPickerEvent& event )
{
  wxString newPath(event.GetPath());

  fileString->assign(newPath.char_str());
}


FileField::FileField(wxWindow* parent, wxWindowID id, const wxString& name, string* _fileString)
    : wxPanel(parent,wxID_ANY), fileString(_fileString)
{
  wxBoxSizer* epiDataSizer = new wxBoxSizer(wxHORIZONTAL);
  wxStaticText* staticText = new wxStaticText(this,wxID_ANY,name);
  wxFilePickerCtrl* epiDataEntry = new wxFilePickerCtrl(this,id,wxString::FromAscii(fileString->c_str()),wxT("Select a file"),wxT("*.*"),wxDefaultPosition,wxDefaultSize,wxFLP_USE_TEXTCTRL | wxFLP_CHANGE_DIR);
  epiDataSizer->Add(staticText,0,wxALIGN_CENTER_VERTICAL);
  epiDataSizer->Add(epiDataEntry,1,wxALIGN_CENTER_VERTICAL | wxALL,5);

  Connect(id, wxEVT_COMMAND_FILEPICKER_CHANGED, wxFileDirPickerEventHandler(FileField::OnChange));

  SetSizer(epiDataSizer);

}



void FileField::OnChange( wxFileDirPickerEvent& event )
{
  wxString newPath(event.GetPath());
  fileString->assign(newPath.char_str());
}



OptionPanel::OptionPanel(wxWindow* parent)
    : wxPanel(parent,wxID_ANY)
{
  wxStaticText* staticText;
  wxBoxSizer* boxSizer;

  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

  // Population data
  wxStaticBoxSizer* popDataBox = new wxStaticBoxSizer(wxVERTICAL,this,wxT("Population data"));

  // Population Size
  boxSizer = new wxBoxSizer(wxHORIZONTAL);
  staticText = new wxStaticText(this, wxID_ANY, wxT("Population &size:"));
  wxTextCtrl* popEntry = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize,0,
                                        GenValidator(wxFILTER_NUMERIC, &(config.nTotal)));
  boxSizer->Add(staticText,0,wxALIGN_CENTER_VERTICAL);
  boxSizer->Add(popEntry,0,wxEXPAND |wxALIGN_CENTER_VERTICAL | wxALL, 5);
  popDataBox->Add(boxSizer,0,wxEXPAND | wxALL, 5);

  // Observation
  boxSizer = new wxBoxSizer(wxHORIZONTAL);
  staticText = new wxStaticText(this, wxID_ANY, wxT("Observation &time:"));
  wxTextCtrl* obsTime = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize,0,
                                       GenValidator(wxFILTER_NUMERIC, &(config.obsTime)));
  boxSizer->Add(staticText,0,wxALIGN_CENTER_VERTICAL);
  boxSizer->Add(obsTime,0,wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 5);
  popDataBox->Add(boxSizer,0,wxEXPAND | wxALL, 5);


  // Runtime options
  wxStaticBoxSizer* runtimeBox = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Runtime options"));
  wxGridSizer* runtimeGrid = new wxGridSizer(4,5,5);

  // Add mult ratio
  staticText = new wxStaticText(this, wxID_ANY, wxT("Add/Mult &ratio:"));
  wxTextCtrl* amRatio = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize,0,
                                       GenValidator(wxFILTER_NUMERIC,&(config.addMultRatio)));
  runtimeGrid->Add(staticText,0,wxALIGN_CENTER_VERTICAL  | wxALIGN_RIGHT | wxALL,0);
  runtimeGrid->Add(amRatio,1,wxALIGN_CENTER_VERTICAL | wxALL,0);

  // Add/mult offset
  staticText = new wxStaticText(this, wxID_ANY, wxT("Add/Mult offset:"));
  wxTextCtrl* addMultOffset = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize,0,
      GenValidator(wxFILTER_NUMERIC,&(config.addMultOffset)));
  runtimeGrid->Add(staticText,0,wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxALL, 0);
  runtimeGrid->Add(addMultOffset, 1, wxALIGN_CENTER_VERTICAL | wxALL, 0);

  // Num iterations
  staticText = new wxStaticText(this, wxID_ANY, wxT("MCMC iterations:"));
  wxTextCtrl* numIterations = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize,0,
      GenValidator(wxFILTER_NUMERIC,&(config.maxIter)));
  runtimeGrid->Add(staticText,0,wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxALL,0);
  runtimeGrid->Add(numIterations, 1, wxALIGN_CENTER_VERTICAL | wxALL,0);

  // Burnin
  staticText = new wxStaticText(this, wxID_ANY, wxT("Burn-in:"));
  wxTextCtrl* burnIn = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize,0,
                                      GenValidator(wxFILTER_NUMERIC,&(config.burnIn)));
  runtimeGrid->Add(staticText,0,wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxALL,0);
  runtimeGrid->Add(burnIn, 1, wxALIGN_CENTER_VERTICAL | wxALL,0);

  // Thin
  staticText = new wxStaticText(this, wxID_ANY, wxT("Thin:"));
  wxTextCtrl* thinBy = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize,0,
                                      GenValidator(wxFILTER_NUMERIC,&(config.thinBy)));
  runtimeGrid->Add(staticText,0,wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT  | wxALL,0);
  runtimeGrid->Add(thinBy,1,wxALIGN_CENTER_VERTICAL | wxALL,0);

  // Number of I moves
  staticText = new wxStaticText(this, wxID_ANY, wxT("# I moves:"));
  wxTextCtrl* numMoves = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize,0,
                                        GenValidator(wxFILTER_NUMERIC,&(config.numImoves)));
  runtimeGrid->Add(staticText,0,wxALIGN_CENTER_VERTICAL  | wxALIGN_RIGHT | wxALL,0);
  runtimeGrid->Add(numMoves, 1, wxALIGN_CENTER_VERTICAL | wxALL,0);

  // Adapt sample frequency
  staticText = new wxStaticText(this, wxID_ANY, wxT("Adapt sample freq:"));
  wxTextCtrl* adaptFreq = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize,0,
                                         GenValidator(wxFILTER_NUMERIC,&(config.adaptSampleFreq)));
  runtimeGrid->Add(staticText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxALL, 0);
  runtimeGrid->Add(adaptFreq, 1, wxALIGN_CENTER_VERTICAL | wxALL, 0);

  // Adapt independent sample frequency
  staticText = new wxStaticText(this, wxID_ANY, wxT("Adapt cov=1 freq:"));
  wxTextCtrl* indentCov = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize,0,
                                         GenValidator(wxFILTER_NUMERIC,&(config.adaptOrthPropFreq)));
  runtimeGrid->Add(staticText, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxALL, 0);
  runtimeGrid->Add(indentCov, 1, wxALIGN_CENTER_VERTICAL | wxALL, 0);

  runtimeBox->Add(runtimeGrid,1,wxEXPAND | wxALL, 10);

  mainSizer->Add(popDataBox,0, wxEXPAND | wxALL,10);
  mainSizer->Add(runtimeBox,0,wxEXPAND | wxALL, 10);

  SetSizer(mainSizer);

  InitDialog();

}


