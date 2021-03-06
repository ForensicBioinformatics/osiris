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
*  FileName: CDialogAcceptAllele.h
*  Author:   Douglas Hoffman
*
*/
#ifndef __C_DIALOG_ACCEPT_ALLELE_H__
#define __C_DIALOG_ACCEPT_ALLELE_H__

#include <wx/dialog.h>
#include <wx/string.h>
#include "COARmessage.h"

class wxSplitterWindow;
class CGridAlerts;
class CGridLocusPeaks;
class COARlocus;
class CAppendAcceptanceLocus;
class CDialogAcceptAllele : public wxDialog
{
public:
  CDialogAcceptAllele(
    COARlocus *pLocus,
    const COARmessages *pMessages,
    wxWindow *parent,
    wxWindowID id,
    bool bAllowUserOverride,
    const wxString &sSampleName,
    const wxSize &sz = wxDefaultSize);
  virtual ~CDialogAcceptAllele();
  void OnEdit(wxCommandEvent &e);
private:
  COARmessages m_MsgEdit;
  wxSplitterWindow *m_pSplitter;
  CGridAlerts *m_pGridAlerts;
  CGridLocusPeaks *m_pPeaks;
  CAppendAcceptanceLocus *m_pRev;
  DECLARE_EVENT_TABLE()
};
#endif
