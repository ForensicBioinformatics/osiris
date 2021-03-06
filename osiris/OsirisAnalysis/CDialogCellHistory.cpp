/*
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*  FileName: CDialogCellHistory.cpp
*  Author:   Douglas Hoffman
*
*/
#include "mainApp.h"
#include <wx/sizer.h>
#include "CDialogCellHistory.h"
#include "CPanelSampleAlertNotebook.h"
#include "CPanelLocusDetails.h"
#include "wxIDS.h"
#include "CSplitterCellHistory.h"

void CDialogCellHistory::_Setup(CSplitterCellHistory *pSplitter)
{
  wxBoxSizer *pSizer = new wxBoxSizer(wxVERTICAL);
  pSizer->Add(pSplitter,1,wxEXPAND);
  pSizer->Add(CreateButtonSizer(wxOK),0,wxALL | wxALIGN_CENTRE,ID_BORDER);
  SetSizer(pSizer);
  pSizer->Layout();
  pSplitter->Split();
}


CDialogCellHistory::CDialogCellHistory(
    const COARlocus &Locus,
    const COARmessages &Msg,
    const set<wxDateTime> &setDateTime,
    const wxString &sSampleName,
    wxWindow *parent, 
    wxWindowID id,
    const wxSize &sz)
    : wxDialog(parent,id,wxEmptyString,
        wxDefaultPosition,sz,
        mainApp::DIALOG_STYLE)
{
  CSplitterCellHistory *pSplitter = 
    new CSplitterCellHistory(
      Locus,
      Msg,
      setDateTime,
      this, 
      IDhistoryPanel);
  wxString sTitle(_T("Allele Edit History for "));
  sTitle.Append(Locus.GetName());
  if(!sSampleName.IsEmpty())
  {
    sTitle.Append(_T(" on "));
    sTitle.Append(sSampleName);
  }
  SetTitle(sTitle);
  _Setup(pSplitter);
}


CDialogCellHistory::CDialogCellHistory(
    const COARsample &Sample,
    int nChannel,
    const COARlocus &Locus,
    const COARmessages &Msg,
    const set<wxDateTime> &setDateTime,
    wxWindow *parent, 
    wxWindowID id,
    const wxSize &sz)
    : wxDialog(parent,id,wxEmptyString,
        wxDefaultPosition,sz,
        mainApp::DIALOG_STYLE)
{
  CSplitterCellHistory *pSplitter = 
    new CSplitterCellHistory(
      Sample,
      nChannel,
      Locus,
      Msg,
      setDateTime,
      this, 
      IDhistoryPanel);
  wxString sTitle(_T("Peak Edit History for "));
  sTitle.Append(Locus.GetName());
  const wxString &sSampleName(Sample.GetName());
  if(!sSampleName.IsEmpty())
  {
    sTitle.Append(" on ");
    sTitle.Append(sSampleName);
  }
  SetTitle(sTitle);
  _Setup(pSplitter);
}
CDialogCellHistory::CDialogCellHistory(
    int nSelect,
    const COARfile &File,
    const COARsample &Sample,
    const map<int,wxString> &mapChannelNames,
    wxWindow *parent,
    wxWindowID id,
    const wxSize &sz)
    : wxDialog(parent,id,wxEmptyString,
        wxDefaultPosition,sz,
        mainApp::DIALOG_STYLE)
{
  CSplitterCellHistory *pSplitter = 
    new CSplitterCellHistory(
      nSelect,
      false,
      File,
      Sample,
      mapChannelNames,
      this,
      IDhistoryPanel);
  wxString s(_T("Alert History for "));
  s.Append(Sample.GetName());
  SetTitle(s);
  _Setup(pSplitter);
}
