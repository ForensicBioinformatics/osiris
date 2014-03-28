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
*
*  FileName: CPanelPlot.h
*  Author:   Douglas Hoffman
*
*/
#include "mainApp.h"
#include <wx/tglbtn.h>
#include "nwx/nwxKeyState.h"
#include "CPanelPlot.h"
#include "CFramePlot.h"
#include "CFrameAnalysis.h"
#include "CPlotData.h"
#include "COARfile.h"
#include "CKitColors.h"
#include "nwx/nwxString.h"
#include "CDialogParameters.h"
#include "nwx/vectorptr.h"
#include "nwx/mapptr.h"

#define RESIDUAL_THRESHOLD 0.0005
#define FIT _T("\nFit: ")

CPlotCtrl::CPlotCtrl(
  wxWindow *parent, CPanelPlot *pPlot, wxWindowID id) 
    : nwxPlotCtrl(parent,id) , m_pPlot(pPlot) {}

void CPlotCtrl::OnClickXLabel(const nwxPointLabel &x, const wxPoint &)
{
  m_pPlot->ZoomToLocus(x.GetLabel(),0);
  return;
}
bool CPlotCtrl::SetViewRect(
  const wxRect2DDouble &view, bool send_event)
{
  bool bRtn = nwxPlotCtrl::SetViewRect(view,send_event);
  m_pPlot->UpdateLadderLabels();
  return bRtn;
}



CPanelPlot::CPanelPlot(
  wxWindow *parent,
  CFrameAnalysis *pFrame, 
  CPlotData *pData, 
  COARfile *pFile, 
  CKitColors *pColors, 
  bool bExternalTimer) : 
    wxSashWindow(parent,wxID_ANY,
      wxDefaultPosition, wxDefaultSize, 0),
    m_pData(pData),
    m_pOARfile(pFile),
    m_pColors(pColors),
    m_pPlotCtrl(NULL),
    m_pButtonPanel(NULL),
    m_pMenu(NULL),
#if !REUSE_MENUS
    m_pMenuPopup(NULL),
#endif
    m_pShiftSizer(NULL),
    m_pFramePlot(NULL),
    m_pFrameAnalysis(pFrame),
    m_pTimer(NULL),
    m_pMenuItem(NULL),
    m_nPlotNr(0),
    m_nBatchCount(0),
    m_nILScurveOffset(0),
    m_nNoiseCurves(0),
    m_bExternalTimer(bExternalTimer)
{
  //  constructor for panel in analysis MDI frame
  _BuildPanel(0,true,NULL);
}


CPanelPlot::CPanelPlot(
  CFramePlot *pFrame,
  CPlotData *pData,
  COARfile *pFile,
  CMenuHistory *pMenuHistory,
  CKitColors *pColors,
  int nMenuNumber,
  bool bFirst,
  int nPlotNumber,
  bool bExternalTimer) :
    wxSashWindow(pFrame->GetPanel(),wxID_ANY,
      wxDefaultPosition,wxDefaultSize,wxSW_3DSASH),
    m_pData(pData),
    m_pOARfile(pFile),
    m_pColors(pColors),
    m_pPlotCtrl(NULL),
    m_pButtonPanel(NULL),
    m_pMenu(NULL),
#if !REUSE_MENUS
    m_pMenuPopup(NULL),
#endif
    m_pShiftSizer(NULL),
    m_pFramePlot(pFrame),
    m_pFrameAnalysis(NULL),
    m_pTimer(NULL),
    m_pMenuItem(NULL),
    m_nPlotNr(nPlotNumber),
    m_nBatchCount(0),
    m_nILScurveOffset(0),
    m_nNoiseCurves(0),
    m_bExternalTimer(bExternalTimer)
{
  //  constructor for panel in graphic MDI frame
  _BuildPanel(nMenuNumber,bFirst,pMenuHistory);
}
void CPanelPlot::_BuildPanel(
  int nMenuNumber,bool bFirst, 
  CMenuHistory *pMenuHistory)
{
  _BuildMenu(nMenuNumber);
  //  initialize
  double *pdx;
  double *pdy;
  unsigned int i;

  size_t nPoints = m_pData->GetPointCount();

  unsigned int nChannelCount = m_pData->GetChannelCount();
  for(i = 0; i < COUNT_DATA; i++)
  {
    m_pmapChannelPlot[i] = NULL;
    m_pmapChannelPlotNoise[i] = NULL;
  }
  // set up data

  pdx = m_pData->GetTimePoints();
  for(i = 1; i <= nChannelCount; i++)
  {
    pdy = m_pData->GetAnalyzedPoints(i);
    AddData(ANALYZED_DATA,i,(unsigned int)nPoints,pdx,pdy);
    pdy = m_pData->GetRawPoints(i);
    AddData(RAW_DATA,i,(unsigned int)nPoints,pdx,pdy);
    pdy = m_pData->GetLadderPoints(i);
    AddData(LADDER_DATA,i,(unsigned int)nPoints,pdx,pdy);
    pdy = m_pData->GetBaselinePoints(i);
#if BASELINE_START
    if(pdy != NULL)
    {
      double *pdxBaseline = m_pData->GetBaselineTimePoints(i);
      AddData(
          BASELINE_DATA,i,
          (unsigned int) m_pData->GetBaselinePointCount(i),
          pdxBaseline,pdy, 
          m_pData->GetBaselineStart(i));
    }
#else
    AddData(BASELINE_DATA,i,(unsigned int)nPoints,pdx,pdy);
#endif
  }

  // now build it and they will come
  m_pPanel = new wxPanel(this,wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);

  if(m_pFramePlot != NULL)
  {
    m_pButtonPanel = new CPanelPlotToolbar(m_pPanel,m_pData,m_pColors,pMenuHistory,bFirst);
    m_pButtonPanel->CopySettings(*m_pMenu);
    m_pShiftSizer = new nwxShiftSizer(
      m_pButtonPanel,this,ID_BORDER,250,true);
  }

  m_pPlotCtrl = new CPlotCtrl(m_pPanel,this);
  m_viewRect.SetPlotCtrl(m_pPlotCtrl);

//  if(m_pFramePlot == NULL)
//  {
//    m_pPlotCtrl->ShowScrollbars(false);
//  }

  TnwxBatch<CPanelPlot> batch(this); // must be after m_pPlotCtrl is created

  m_pPlotCtrl->SetPlotTitleColour(*wxWHITE);
  m_pPlotCtrl->SetPlotTitle(_T("M|W")); // pick large characters
  m_pPlotCtrl->SetShowPlotTitle(true);

  m_pSizer = new wxBoxSizer(wxVERTICAL);
  if(m_pShiftSizer != NULL)
  {
    m_pSizer->Add(m_pShiftSizer,0,wxEXPAND);
  }
  m_pSizer->Add(m_pPlotCtrl,1,wxEXPAND);

  m_pPlotCtrl->SetDrawSymbols(false);
  m_pPlotCtrl->SetXAxisLabel(_T("Time (seconds)"));
  m_pPlotCtrl->SetYAxisLabel(_T("RFU"));
  m_pPlotCtrl->SetShowXAxisLabel(true);
  m_pPlotCtrl->SetShowYAxisLabel(true);
  m_pPlotCtrl->SetMinExpValue(99999);
  m_pPanel->SetSizer(m_pSizer);
  m_pPanel->Layout();
}

void CPanelPlot::_CleanupLadderPeakSet()
{
  vectorptr<CLadderPeakSet>::cleanup(&m_vpLadderPeakSet);
}

CPanelPlot::~CPanelPlot()
{
  mapChannelPlot *pcp;
  mapChannelPlot::iterator itr;
  mapChannelPlot **pp = m_pmapChannelPlotNoise;
  m_pPlotCtrl->DeleteCurve(-1);
  for(int zz = 0; zz < 2; zz++)
  {
    for(int i = 0; i < COUNT_DATA; i++)
    {
      pcp = pp[i];
      if(pcp != NULL)
      {
        pp[i] = NULL;
        mapptr<int,wxPlotData>::cleanup(pcp);
        delete pcp;
      }
    }
    pp = m_pmapChannelPlot;
  }
  vectorptr<wxPlotData>::cleanup(&m_vILS);
  this->_CleanupLadderPeakSet();
  if(m_pTimer != NULL)
  {
    delete m_pTimer;
    m_pTimer = NULL;
  }
  CleanupMinRfuLines();

  if(m_pMenuItem.get() == NULL)
  {
    delete m_pMenu;
    m_pMenu = NULL;
  }
#if !REUSE_MENUS
  if(m_pMenuPopup != NULL) { delete m_pMenuPopup;}
#endif
}

void CPanelPlot::CleanupMinRfuLines()
{
  for(mapMinRfu::iterator itr = m_mapMinRfuAll.begin();
    itr != m_mapMinRfuAll.end();
    ++itr)
  {
    delete itr->second;
  }
  m_mapMinRfuAll.clear();
}

void CPanelPlot::AddData(
  DATA_TYPE nType, unsigned int nChannel, unsigned int nPointCount,
  double *pdx, double *pdy, unsigned int nStart)
{
  if((pdy != NULL) && (pdx != NULL) && (nPointCount > 1))
  {
    unsigned int nBegin = m_pData->GetBegin();
    if(!nStart){}
    else if(nBegin < nStart)
    {
      nBegin = 0;
    }
    else
    {
      nBegin -= nStart;
    }

    if((nBegin + 1) >= nPointCount)
    {
      nBegin = 0;
    }
    mapChannelPlot *pcp = m_pmapChannelPlot[nType];
    mapChannelPlot *pcpNoise = m_pmapChannelPlotNoise[nType];
    if(pcpNoise == NULL)
    {
      // change 4/4/13, move if(nBegin) inside this 'if' instead of outside if/else
      if(nBegin)
      {
        pcpNoise = new mapChannelPlot;
        m_pmapChannelPlotNoise[nType] = pcpNoise;
      }
    }
    else
    {
      mapChannelPlot::iterator itr = pcpNoise->find(nChannel);
      if(itr != pcpNoise->end())
      {
        delete itr->second;
        pcpNoise->erase(itr);
      }
    }
    if(pcp == NULL)
    {
      pcp = new mapChannelPlot;
      m_pmapChannelPlot[nType] = pcp;
    }
    else
    {
      mapChannelPlot::iterator itr = pcp->find(nChannel);
      if(itr != pcp->end())
      {
        delete itr->second;
        pcp->erase(itr);
      }
    }


    wxPlotData *pPlotData = new wxPlotData(&pdx[nBegin],&pdy[nBegin],(int)(nPointCount - nBegin),true);
    pcp->insert(mapChannelPlot::value_type(nChannel,pPlotData));

    wxGenericPen pen(
      m_pColors->GetColor(
            m_pData->GetKitName(),nType,nChannel));
    pPlotData->SetPen(wxPLOTPEN_NORMAL,pen);
    pPlotData->SetPen(wxPLOTPEN_ACTIVE,pen);
    pPlotData->SetPen(wxPLOTPEN_SELECTED,pen);
    if(nBegin)
    {
      pPlotData = new wxPlotData(pdx,pdy,nBegin + 1,true);
      pcpNoise->insert(mapChannelPlot::value_type(nChannel,pPlotData));
      pPlotData->SetPen(wxPLOTPEN_NORMAL,pen);
      pPlotData->SetPen(wxPLOTPEN_ACTIVE,pen);
      pPlotData->SetPen(wxPLOTPEN_SELECTED,pen);
    }
  }
}

wxPlotData *CPanelPlot::FindData(DATA_TYPE nType, unsigned int nChannel, bool bNoise)
{
  wxPlotData *pRtn(NULL);
  mapChannelPlot *pcp = bNoise ? m_pmapChannelPlotNoise[nType] : m_pmapChannelPlot[nType];
  if(pcp != NULL)
  {
    mapChannelPlot::iterator itr = pcp->find(nChannel);
    if(itr != pcp->end())
    {
      pRtn = itr->second;
    }
  }
  return pRtn;
}
void CPanelPlot::OnBtnAppend(wxCommandEvent &)
{
  if(m_pFramePlot != NULL)
  {
//    wxRect2DDouble rect = GetViewRect();
    m_pFramePlot->AddPlot(this);
//    SetViewRect(rect);
  }
}

void CPanelPlot::OnBtnMultiple(wxCommandEvent &e)
{
  if(m_pFramePlot != NULL)
  {
    int nID = e.GetId();
    bool bShift = false;
    if(!ID_IS_MENU_PLOT(nID))
    {
      bShift = nwxKeyState::Shift();
    }
    if(bShift)
    {
      m_pFramePlot->RemoveAllPlotsExcept(this);
    }
    else
    {
      m_pFramePlot->SetMultiple(this,true);
      m_pFramePlot->Refresh();
    }
  }
}
void CPanelPlot::OnBtnRemove(wxCommandEvent &e)
{
  if(m_pFramePlot != NULL)
  {
    int nID = e.GetId();
    bool bRemoveExceptThis = false;

    if(ID_IS_MENU_PLOT(nID))
    {
      bRemoveExceptThis =
        ( (nID - m_nMenuOffset) == IDmenuPlotRemoveOthers );
    }
    else
    {
      bRemoveExceptThis = nwxKeyState::Shift();
    }
    if(bRemoveExceptThis)
    {
      m_pFramePlot->RemoveAllPlotsExcept(this);
    }
    else
    {
      m_pFramePlot->RemovePlot(this);
    }
  }
}
void CPanelPlot::OnBtnDetails(wxCommandEvent &)
{
  CMDIFrame *pParent = 
    (m_pFramePlot != NULL) 
    ? (CMDIFrame *) m_pFramePlot 
    : (CMDIFrame *) m_pFrameAnalysis;


  CLabSettings *pLab(NULL);
  CXMLmessageBook *pMsgBook(NULL);
  if(m_pOARfile != NULL)
  {
    pLab = &m_pOARfile->GetLabSettings();
    pMsgBook = m_pOARfile->GetMessageBook();
  }

  const CParmOsiris &parm(m_pData->GetParameters());
  const wxString &sLadder(m_pData->GetLadder());
  const wxString &sFileName(m_pData->GetFilename());
  CDialogParameters dlg(
    pParent,&parm,&sFileName,&sLadder,pLab,pMsgBook);
  dlg.ShowModal();
}
void CPanelPlot::OnSync(wxCommandEvent &e)
{
  if(m_pFramePlot != NULL)
  {
    int nID = e.GetId();
    CPlotController *pControl = NULL;
    if(ID_IS_MENU_PLOT(nID))
    {
      pControl = _GetLastMenu();
    }
    else
    {
      pControl = m_pButtonPanel;
    }
    if(pControl != NULL)
    {
      _SyncControllers(pControl);
    }
    if(SyncValue())
    {
      m_pFramePlot->SyncThis(this);
    }
    if(nwxKeyState::Shift())
    {
      m_pFramePlot->SyncState(this,IDmenuPlotSync);
    }

  }
}

void CPanelPlot::OnZoomOut(wxCommandEvent &e)
{
  bool bFull = false;
  int nID = e.GetId();
  if(ID_IS_MENU_PLOT(nID))
  {
    bFull = ((nID - m_nMenuOffset) == IDmenuPlotResetAxesFull);
  }
  else
  {
    bFull = nwxKeyState::Shift();
  }
  if((m_pFramePlot != NULL) && SyncValue())
  {
    SetViewRect(m_pFramePlot->GetZoomOutRect(bFull));
  }
  else
  {
    ZoomOut(bFull);
  }
  Refresh();
}
void CPanelPlot::OnRebuildCurves(wxCommandEvent &e)
{
  bool bShift = nwxKeyState::Shift();
  bool bHasToolbar = HasToolbar();
  wxObject *pObj = bShift ? e.GetEventObject() : NULL;
  unsigned int nChannelButton = 0;
  if(bHasToolbar)
  {
    if(bShift)
    {
      nChannelButton = m_pButtonPanel->GetChannelFromButton(pObj);
      if(nChannelButton)
      {
        ShowOneChannel(nChannelButton);
      }
    }
    _SyncControllers(m_pButtonPanel);
  }
  RebuildCurves();
  if( (!nChannelButton) && bShift && bHasToolbar && (m_pFramePlot != NULL) )
  {
    int nID = 0;

    if(m_pButtonPanel->IsButtonAnalyzed(pObj))
    {
      nID = IDmenuPlotDataAnalyzed;
    }
    else if(m_pButtonPanel->IsButtonRaw(pObj))
    {
      nID = IDmenuPlotDataRaw;
    }
    else if(m_pButtonPanel->IsButtonBaseline(pObj))
    {
      nID = IDmenuPlotDataBaseline;
    }
    else if(m_pButtonPanel->IsButtonLadder(pObj))
    {
      nID = IDmenuPlotDataLadder;
    }
    else if(m_pButtonPanel->IsButtonRfu(pObj))
    {
      nID = IDmenuPlotRFU;
    }
    else if(m_pButtonPanel->IsButtonLadderLabels(pObj))
    {
      nID = IDmenuPlotLadderLabels;
    }
    else if(m_pButtonPanel->IsButtonILS(pObj))
    {
      nID = IDmenuPlotILS;
    }
    if(nID)
    {
      m_pFramePlot->SyncState(this,nID);
    }
  }
  UpdateSettings();
}



void CPanelPlot::BuildILSlines()
{
  unsigned int nChannelILS = m_pData->GetChannelCount();
  if( (!m_vILS.size()) &&  nChannelILS )
  {
    double Y[2] = {-60000.0, 60000.0};
    double dx[2] = {0.0,0.0};
    double *pdx;
    double *pdy;
    wxGenericPen pen(
      m_pColors->GetColor(
        m_pData->GetKitName(),LADDER_DATA,nChannelILS));
    const vector<CSamplePeak *> *pvILS =
      m_pData->GetSamplePeaks(nChannelILS);
    for(vector<CSamplePeak *>::const_iterator itr = pvILS->begin();
      itr != pvILS->end();
      ++itr)
    {
      dx[0] = (*itr)->GetMeanDouble();
      dx[1] = dx[0];
      pdx = Copy2Points(dx);
      pdy = Copy2Points(Y);
      wxPlotData *pPlotData =
        new wxPlotData(pdx,pdy,2,false);
      m_vILS.push_back(pPlotData);
      pPlotData->SetPen(wxPLOTPEN_NORMAL,pen);
      pPlotData->SetPen(wxPLOTPEN_ACTIVE,pen);
      pPlotData->SetPen(wxPLOTPEN_SELECTED,pen);
    }
  }
}

void CPanelPlot::SetOARfile(COARfile *pFile)
{
  if(pFile != m_pOARfile)
  {
    m_pOARfile = pFile;
    SetupLabelMenus();
    LabelTypeChanged();
  }
}

wxString CPanelPlot::_AlleleLabel(
  const IOARpeak *pPeak, LABEL_PLOT_TYPE nType)
{
  CParmOsirisGlobal parm;
  parm->GetGlobal();
  wxString sLabel;
  vector<wxString> *selection;
  switch(nType)
  {
  case LABEL_MULTIPLE:
	  sLabel = _T("");
	  selection = new vector<wxString>();
	  selection->resize(5);
	  if (CanShowPeakArea() && parm->GetMultiLabelPeakArea()) selection->push_back(wxT("PA: ") + nwxString::FormatNumber(pPeak->GetPeakArea()));
	  if (parm->GetMultiLabelTime()) selection->push_back(wxT("TI: ") + nwxString::FormatNumber(pPeak->GetTime()));
	  if (parm->GetMultiLabelRFU()) selection->push_back(wxT("RF: ") + nwxString::FormatNumber(nwxRound::Round(pPeak->GetRFU())));
	  if (parm->GetMultiLabelBPS()) selection->push_back(wxT("BP: ") + nwxString::FormatNumber(nwxRound::Round(pPeak->GetBPS())));
	  if (parm->GetMultiLabelAllele()) selection->push_back(wxT("AL: ") + COARpeak::FormatAlleleName(*pPeak, COARlocus::IsAmel(pPeak->GetLocusName()), true));
	  for (unsigned int i = 0; i < selection->size(); i++) {
		  sLabel += selection->back();
		  if (parm->GetMultiLineLabel() == false && selection->back() != wxEmptyString) sLabel += wxT("; "); else sLabel += wxT("\n");
		  selection->pop_back();
	  }
	  break;
  case LABEL_ALLELE:
    sLabel = COARpeak::FormatAlleleName(
      *pPeak,
      COARlocus::IsAmel(pPeak->GetLocusName()),
      true);
    break;
  case LABEL_BPS:
    sLabel = nwxString::FormatNumber(
      nwxRound::Round(pPeak->GetBPS()) );
    break;
  case LABEL_RFU:
    sLabel = nwxString::FormatNumber(
      nwxRound::Round(pPeak->GetRFU()) );
    break;
  case LABEL_TIME:
    sLabel = nwxString::FormatNumber(pPeak->GetTime());
    break;
  case LABEL_PEAK_AREA:
    sLabel = nwxString::FormatNumber(
      pPeak->GetPeakArea());
    break;
  default:
    sLabel = _T("Problem");
    break;
  }
  return sLabel;
}
wxString CPanelPlot::_ArtifactToolTip(const IOARpeak *pPeak, const wxString &sChannelName)
{
  wxString sToolTip;
  double dBPS = pPeak->GetMeanBPS();

  sToolTip += wxString::Format(
      "Artifact: %s\n"
      "RFU: %d\n"
      "Time: %g\n",
      pPeak->GetArtifactLabel().c_str(),
      nwxRound::Round(pPeak->GetRFU()),
      pPeak->GetTime());
  if(dBPS > 1.0)
  {
    sToolTip += wxString::Format("ILS Ref.: %.2f\n",dBPS);
  }
  sToolTip += wxString::Format("Channel: %s",sChannelName.c_str());
  if(!pPeak->IsAllele())
  {
    wxString sLocus = pPeak->GetLocusName();
    wxString sAllele = pPeak->GetAlleleName();
    if(pPeak->IsOffLadder())
    {
      sAllele.Append(_T(OFF_LADDER));
    }
    else if(pPeak->IsOffLadderAccepted())
    {
      sAllele.Append(_T(OFF_LADDER_ACCEPTED));
    }

    if(!( sLocus.IsEmpty() || sAllele.IsEmpty() ))
    {
      double dBPSresid = pPeak->GetBPS();
      int nBPS = nwxRound::Round(dBPSresid);
      sToolTip += wxString::Format(
        _T("\nLocus: %s\n"
           "Allele: %s"),
           sLocus.c_str(),
           sAllele.c_str()
           );
      if(nBPS)
      {
        sToolTip += wxString::Format(
          _T("\nAllele BPS: %d"),
           nBPS);

        dBPSresid -= (double)nBPS;
        if( dBPSresid > RESIDUAL_THRESHOLD || dBPSresid < -RESIDUAL_THRESHOLD )
        {
          sToolTip += wxString::Format(
            _T("\nResidual: %.3f"),dBPSresid);
        }
      }
    }
  }
  if(pPeak->GetFit() >= 0.0)
  {
    sToolTip += FIT;
    sToolTip += COARpeak::FormatFit(*pPeak);
  }
  return sToolTip;
}
const char *CPanelPlot::OFF_LADDER("\n    Off Ladder");
const char *CPanelPlot::OFF_LADDER_ACCEPTED("\n    Accepted Off Ladder");
wxString CPanelPlot::_AlleleToolTip(
  const IOARpeak *pPeak, int nChannel, const wxString &sChannelName)
{
  wxString sToolTip;
  double dTime = pPeak->GetTime();
  sToolTip.Alloc(1024);

  wxString sLocus = pPeak->GetLocusName();
  if(sLocus.IsEmpty())
  {
    sLocus =
      m_pData->GetLocusName(
      nChannel, nwxRound::Round(dTime) );
  }
  bool bAmel = COARlocus::IsAmel(sLocus);
  wxString sAllele =
    COARpeak::FormatAlleleName(*pPeak,bAmel,false);
  if(pPeak->IsOffLadder())
  {
    sAllele.Append(_T(OFF_LADDER));
  }
  else if(pPeak->IsOffLadderAccepted())
  {
    sAllele.Append(_T(OFF_LADDER_ACCEPTED));
  }

  sToolTip = wxString::Format(
      "Channel: %s\n",
      sChannelName.c_str());
  if(!sLocus.IsEmpty())
  {
    sToolTip += wxString::Format(
        "Locus: %s\n",
        sLocus.c_str());
  }
  double dBPS = pPeak->GetBPS();
  int nBPS = nwxRound::Round(dBPS);
  double dBPSresid = dBPS - (double)nBPS;
  sToolTip += wxString::Format(
      "Allele: %s\n"
      "BPS: %d\n",
    sAllele.c_str(),
    nBPS
    );

  //  The 'if' may be temporary
  
  if( dBPSresid > RESIDUAL_THRESHOLD || dBPSresid < -RESIDUAL_THRESHOLD )
  {
    sToolTip += wxString::Format(
//        "BPS w/ resid: %.3f\n"
        "Residual: %.3f\n",
//      dBPS,
      dBPSresid);
  }

  sToolTip += wxString::Format(
      "RFU: %d\n"
      "Time: %g",
    nwxRound::Round(pPeak->GetRFU()),
    dTime);

  double dILSref = pPeak->GetMeanBPS();
  if(dILSref > 1.0)
  {
    sToolTip += wxString::Format("\nILS Ref.: %.2f",dILSref);
  }

  if(pPeak->GetFit() >= 0.0)
  {
    sToolTip += FIT;
    sToolTip += COARpeak::FormatFit(*pPeak);
  }
  return sToolTip;
}

void CPanelPlot::_BuildPeakLabels(
  const vector<CSamplePeak *> *pp,
  const wxColour &colour,
  const wxString &sChannelName,
  unsigned int nChannel,
  LABEL_PLOT_TYPE nLabelType)
{
  wxString sLabel;
  wxString sToolTip;
  size_t n = 
    ((nLabelType == LABEL_NONE) || (pp == NULL))
    ? 0
    : pp->size();
  size_t j;
  for(j = 0; j < n; j++)
  {
    const CSamplePeak *pPeak = pp->at(j);
    sLabel = _AlleleLabel(pPeak,nLabelType);
    sToolTip = _AlleleToolTip(pPeak,nChannel,sChannelName);
    nwxPointLabel label(
            sLabel,
            pPeak->GetTime(),
            pPeak->GetRFU(),
            colour,
            sToolTip,
            wxALIGN_CENTRE_HORIZONTAL | wxALIGN_BOTTOM);
    m_pPlotCtrl->AddLabel(label);
  }
}

int CPanelPlot::_GetLadderPeakCount()
{
  int nRtn = 0;
  vector<CLadderPeakSet *>::iterator itr;
  for(itr = m_vpLadderPeakSet.begin();
    itr != m_vpLadderPeakSet.end();
    ++itr)
  {
    nRtn += (int) (*itr)->GetPeakCount();
  }
  return nRtn;
}

void CPanelPlot::_BuildLadderPeakLabels(
  LABEL_PLOT_TYPE nLabelType)
{
  CParmOsirisGlobal parm;
  int nMax = parm->GetMaxLadderLabels();
  if( (nMax < 1) ||
      (_GetLadderPeakCount() <= nMax) )
  {
    vector<CLadderPeakSet *>::iterator itr;
    for(itr = m_vpLadderPeakSet.begin();
      itr != m_vpLadderPeakSet.end();
      ++itr)
    {
      if((*itr)->HasPeaks())
      {
        _BuildPeakLabels(
          (*itr)->GetPeaks(),
          (*itr)->GetColour(),
          (*itr)->GetChannelName(),
          (*itr)->GetChannel(),
          nLabelType);
      }
    }
  }
}

void CPanelPlot::_AppendLadderPeaks(
    unsigned int nChannel,
    const wxString &sChannelName)
{
  const wxColour &colour(m_pColors->GetColor(
    m_pData->GetKitName(),
    //LADDER_DATA,
    ANALYZED_DATA,
    nChannel));
  CLadderPeakSet *pps =
    new CLadderPeakSet(colour,sChannelName,nChannel);
  m_vpLadderPeakSet.push_back(pps);
  const vector<CSamplePeak *> *pp =
      m_pData->GetLadderPeaks(nChannel);
  if( (pp != NULL) && (pp->size() > 0) )
  {
    const wxRect2DDouble &rect = GetViewRect();
    double xMin = rect.GetLeft();
    double xMax = rect.GetRight();
    double dTime;
    vector<CSamplePeak *>::const_iterator itr;
    pps->Reserve(pp->size());
    for(itr = pp->begin(); itr != pp->end(); ++itr)
    {
      dTime = (*itr)->GetTime();
      if(dTime >= xMin && dTime <= xMax)
      {
        pps->AddPeak(*itr);
      }
    }
  }
}

void CPanelPlot::_BuildPLTlabels(bool bArtifactOnly, unsigned int _nChannel)
{
  LABEL_PLOT_TYPE nLabelType = m_pMenu->LabelType();
  if(nLabelType == LABEL_PEAK_AREA)
  {
    nLabelType = LABEL_NONE;
  }
  bool bLabels = (!bArtifactOnly) && (nLabelType != LABEL_NONE);
  bool bLadder = bLabels && m_pMenu->LadderLabels();
  int nArtifact = m_pMenu->ArtifactValue();
  bool bArtifact = (nArtifact > CArtifactDisplayList::nArtifactLabelNone);
  const CSingleKitColors *pKitColors(m_pColors->GetKitColors(m_pData->GetKitName()));
  _CleanupLadderPeakSet();

  if( ( (bLabels || bArtifact) && 
        (m_pMenu->AnalyzedValue() || m_pMenu->RawValue())
      ) ||
      bLadder  
    )
  {
    wxString sLabel;
    wxString sLocus;
    wxString sToolTip;
    wxString sChannelName;
    size_t n;
    size_t j;
    unsigned int nChannelStart = 1;
    unsigned int nChannelEnd = m_pData->GetChannelCount();
    const vector<CSamplePeak *> *pp;
    const vector<CArtifact *> *pa;
    const CChannelColors *pChannelColor;
    if(_nChannel)
    {
      nChannelStart = _nChannel;
      nChannelEnd = _nChannel;
    }

    for(unsigned int nChannel = nChannelStart; nChannel <= nChannelEnd; nChannel++)
    {
      if(m_pMenu->ChannelValue(nChannel))
      {
        const wxColour &colourData(m_pColors->GetColor(
          m_pData->GetKitName(),ANALYZED_DATA,nChannel));

        pp = bLabels
          ? m_pData->GetSamplePeaks(nChannel)
          : NULL;
        pa = bArtifact
          ? m_pData->GetArtifacts(nChannel)
          : NULL;

        pChannelColor =
          (pKitColors == NULL)
          ? NULL
          : pKitColors->GetColorChannel(nChannel);
        sChannelName =
          (pChannelColor == NULL)
          ? wxString::Format(_T("%d"),nChannel)
          : pChannelColor->m_sDyeName;
        if(bLadder)
        {
          _AppendLadderPeaks(nChannel,sChannelName);
        }
        _BuildPeakLabels(
          pp, colourData,  sChannelName,nChannel,nLabelType);
        n = (pa == NULL) ? 0 : pa->size();
        sLabel = _T("A");
        for(j = 0; j < n; j++)
        {
          const CArtifact *pArt = pa->at(j);
          if(pArt->LevelSelected(nArtifact))
          {
            sToolTip = _ArtifactToolTip(pArt,sChannelName);
            nwxPointLabel label(
                  sLabel,
                  pArt->GetTime(),
                  pArt->GetRFU(),
                  colourData,
                  sToolTip,
                  wxALIGN_CENTRE_HORIZONTAL | wxALIGN_BOTTOM);
            m_pPlotCtrl->AddLabel(label);
          }
        }
      }
    }
    if(bLadder)
    {
      _BuildLadderPeakLabels(nLabelType);
      _CleanupLadderPeakSet();
    }
  }
}

void CPanelPlot::_BuildOARlabels()
{
  LABEL_PLOT_TYPE nLabelType = m_pMenu->LabelType();
  bool bLabels = (nLabelType != LABEL_NONE);
  bool bLadderLabels = bLabels && m_pMenu->LadderLabels()
    && m_pMenu->LadderValue();
  int nArtifact = m_pMenu->ArtifactValue();
  bool bArtifact = (nArtifact > CArtifactDisplayList::nArtifactLabelNone);
  const CSingleKitColors *pKitColors(m_pColors->GetKitColors(m_pData->GetKitName()));
  bool bPlot =
    (bLabels || bArtifact) &&
      (m_pMenu->AnalyzedValue() || m_pMenu->RawValue());
  COARsample *pSample =
    bPlot ? m_pOARfile->GetSampleByName(m_pData->GetFilename()) : NULL;

  if(pSample == NULL) {;} // do nothing
  else if(pSample->IsLadderType())
  {
    _BuildPLTlabels();
  }
  else
  {
    wxString sLabel;
    wxString sLocus;
    wxString sToolTip;
    wxString sChannelName;
    const wxString sA(_T("A"));
    size_t n;
    size_t j;
    size_t nChannelCount = (pSample == NULL) ? 0 : m_pData->GetChannelCount();
    const COARchannel *pChannel;
    bool bOARHasArtifacts = bArtifact && m_pOARfile->CanEditArtifacts();

    for(unsigned int nChannel = 1;
      nChannel <= nChannelCount;
      nChannel++)
    {
      if( !m_pMenu->ChannelValue(nChannel) ) {;}
      else if ( (pChannel = m_pOARfile->GetChannelByNr(nChannel)) == NULL )
      {
        _BuildPLTlabels(false,nChannel);
      }
      else
      {
        auto_ptr< vectorptr<COARpeakAny> > pPeaks(
          pSample->GetPeaksByChannel(
            pChannel,GetSelectedTime(),false));
        const CChannelColors *pChannelColor(
          (pKitColors == NULL)
          ? NULL
          :pKitColors->GetColorChannel(nChannel) );
        sChannelName =
          (pChannelColor == NULL)
          ? wxString::Format(_T("%d"),nChannel)
          : pChannelColor->m_sDyeName;
        const wxColour &colour(m_pColors->GetColor(
          m_pData->GetKitName(),ANALYZED_DATA,nChannel));

        n = (pPeaks.get() == NULL) ? 0 : pPeaks->size();
        for(j = 0; j < n; j++)
        {
          COARpeakAny *pPeak = pPeaks.get()->at(j);
          if(bLabels && pPeak->IsAllele())
          {
            sLabel = _AlleleLabel(pPeak,nLabelType);
            sToolTip = _AlleleToolTip(pPeak,nChannel,sChannelName);
            nwxPointLabel label(
                    sLabel,
                    pPeak->GetTime(),
                    pPeak->GetRFU(),
                    colour,
                    sToolTip,
                    wxALIGN_CENTRE_HORIZONTAL | wxALIGN_BOTTOM);
            m_pPlotCtrl->AddLabel(label);
          }
          if(bOARHasArtifacts &&
            pPeak->IsArtifact() &&
            (pPeak->GetCriticalLevel() <= nArtifact)
            )
          {
            sToolTip = _ArtifactToolTip(pPeak,sChannelName);
            nwxPointLabel label(
                  sA,
                  pPeak->GetTime(),
                  pPeak->GetRFU(),
                  colour,
                  sToolTip,
                  wxALIGN_CENTRE_HORIZONTAL | wxALIGN_BOTTOM);
            m_pPlotCtrl->AddLabel(label);
          }
        }
        if(bLadderLabels)
        {
          _AppendLadderPeaks(nChannel,sChannelName);
        }
      }
    }
    if(bLadderLabels)
    {
      _BuildLadderPeakLabels(nLabelType);
      _CleanupLadderPeakSet();
    }
    if(bArtifact && !bOARHasArtifacts)
    {
      // older version of OAR file
      _BuildPLTlabels(true);
    }
  }
}

void CPanelPlot::UpdateLadderLabels()
{
  if( m_pMenu->LadderLabels() &&
      (m_pMenu->LabelType() != LABEL_NONE)
    )
  {
    LabelTypeChanged();
    //RebuildLabels(true);
    //Refresh();
  }
}
void CPanelPlot::RebuildLabels(bool bRedraw)
{
  m_pPlotCtrl->RemoveAllLabels();
  LABEL_PLOT_TYPE nLabelType = m_pMenu->LabelType();
  bool bLabels = (nLabelType != LABEL_NONE);
  int nArtifact = m_pMenu->ArtifactValue();
  bool bArtifact = (nArtifact > CArtifactDisplayList::nArtifactLabelNone);
  if(bLabels || bArtifact)
  {
    if(m_pOARfile != NULL)
    {
      _BuildOARlabels();
    }
    else
    {
      _BuildPLTlabels();
    }
    if(bLabels)
    {
      wxString sToolTip;
      int nx;
      unsigned int nChannel;
      const CPlotLocus *pLocus;
      const vector<CPlotLocus *> *pvLocus = m_pData->GetLoci();
      vector<CPlotLocus *>::const_iterator itr;

      for(itr = pvLocus->begin();
        itr != pvLocus->end();
        ++itr)
      {
        pLocus = *itr;
        nChannel = pLocus->GetChannel();
        if( m_pMenu->ChannelValue(nChannel) )
        {
          const wxColour &colour(m_pColors->GetColor(
            m_pData->GetKitName(),ANALYZED_DATA,nChannel));
          nx = (pLocus->GetStart() + pLocus->GetEnd() + 1) >> 1;
          sToolTip = _T("Click here to zoom to ");
          sToolTip.Append(pLocus->GetName());
          nwxPointLabel label(
            pLocus->GetName(),(double) nx,0.0,colour,sToolTip);
          label.SetToolTip(sToolTip);
          m_pPlotCtrl->AddXLabel(label);
        }
      }
    }
  }
  if(bRedraw)
  {
    Refresh();
    //m_pPlotCtrl->Redraw(wxPLOTCTRL_REDRAW_PLOT);
  }
}

// IShiftReceiver functions
void CPanelPlot::ShiftLeft(bool bShiftKey)
{
  if(HasToolbar())
  {
    m_pButtonPanel->ShiftLeft(bShiftKey);
    if(m_pFramePlot != NULL)
    {
      m_pFramePlot->SyncToolbars(this);
    }
  }
}
void CPanelPlot::ShiftRight(bool bShiftKey)
{
  if(HasToolbar())
  {
    m_pButtonPanel->ShiftRight(bShiftKey);
    if(m_pFramePlot != NULL)
    {
      m_pFramePlot->SyncToolbars(this);
    }
  }
}
bool CPanelPlot::CanShiftLeft()
{
  return 
    HasToolbar() 
    ? m_pButtonPanel->CanShiftLeft()
    : false;
}
bool CPanelPlot::CanShiftRight()
{
  return 
    HasToolbar()
    ? m_pButtonPanel->CanShiftRight()
    : false;
}

const wxDateTime *CPanelPlot::GetSelectedTime()
{
  const wxDateTime *pRtn = NULL;
  if(m_pFramePlot != NULL)
  {
    pRtn = m_pFramePlot->GetSelectedTime();
  }
  else if(m_pFrameAnalysis != NULL)
  {
    pRtn = m_pFrameAnalysis->GetSelectedTime();
  }
  return pRtn;
}
void CPanelPlot::SetExternalTimer(bool b)
{
  m_bExternalTimer = b;
  if( m_bExternalTimer && (m_pTimer != NULL) && (!m_pTimer->IsRunning()) )
  {
    delete m_pTimer;
    m_pTimer = NULL;
  }
}

void CPanelPlot::_OnTimer(wxTimerEvent &e)
{
  if(m_bDoTimer)
  {
    m_bDoTimer = false;
    RebuildCurves();
  }
  if( (m_pShiftSizer != NULL) && m_pShiftSizer->UseExternalTimer() )
  {
    m_pShiftSizer->OnTimer(e);
  }
  m_viewRect.Check();

  // workaround a bug with the 'zoom' cursor
  // if you hold the shift key down and press "Reset Axes" the 
  // cursor is changed to a magnifying glass.
  //  Due to problems using the 'other' mouse functions, just
  //  keep it as zoom.

  const wxPlotCtrlMouse_Type fn(wxPLOTCTRL_MOUSE_ZOOM);
  const int cur(wxCURSOR_CROSS);
  if(m_pPlotCtrl->GetAreaMouseFunction() != fn)
  {
    m_pPlotCtrl->SetAreaMouseFunction(fn,true);
  }
  else if(m_pPlotCtrl->GetAreaMouseCursor() != cur)
  {
    m_pPlotCtrl->SetAreaMouseCursor(cur);
  }
}

void CPanelPlot::OnTimer(wxTimerEvent &e)
{
  if(m_bExternalTimer)
  {
    _OnTimer(e);
  }
}
void CPanelPlot::OnTimerEvent(wxTimerEvent &e)
{
  if(m_bExternalTimer && (m_pTimer != NULL))
  {
    delete m_pTimer;
    m_pTimer = NULL;
  }
  _OnTimer(e);
}
void CPanelPlot::OnPointSelected(wxPlotCtrlEvent &)
{
  if(m_bExternalTimer)
  {
    m_bDoTimer = true;
  }
  else
  {
    if(m_pTimer == NULL)
    {
      m_pTimer = new wxTimer(this,IDtimer);
    }
    m_pTimer->Start(50,true);

  }
}

void CPanelPlot::OnViewChanged(wxPlotCtrlEvent &e)
{
  bool bRefresh = true;
  if(m_nBatchCount || !IsShown())
  {
    bRefresh = false;
  }
  else if( 
      SyncValue() && 
      (m_pPlotCtrl != NULL) && 
      (m_pFramePlot != NULL) &&
      (m_pFramePlot->GetPlotCount() > 1)
      )
  {
    if(!m_pFramePlot->InSyncUpdate())
    {
      m_pFramePlot->SyncTo(this);
    }
    else if(m_pFramePlot->InSyncUpdateTo(this))
    {
      // do this later
      GetEventHandler()->AddPendingEvent(e);
      bRefresh = false;
    }
#if _DEBUG
//    else
//    {
//      bool b = m_pFramePlot->InSyncUpdateTo(NULL);
//    }
#endif
  }
  if(bRefresh)
  {
    m_viewRect.Update();
    LabelTypeChanged();
    Refresh();
  }
}
void CPanelPlot::LabelTypeChanged()
{
  //  this function is called for any change in
  //  labels whether it is the type, atrifact critical level,
  //  or historical time (i.e. to view changes in the file)

  if(m_pPlotCtrl != NULL)
  {
    TnwxBatch<CPanelPlot> batch(this);
    m_pMenu->UsingDefault();

    // if 'using default flag' was set and the user changed the label,
    //  then clear the 'using default flag'

    RebuildLabels();
    Refresh();
  }
}
void CPanelPlot::OnLabelTypeChanged(wxCommandEvent &e)
{
  if(HasToolbar())
  {
    _SyncControllers(m_pButtonPanel);
  }
  LabelTypeChanged();
  UpdateSettings();
  if((m_pFramePlot != NULL) && nwxKeyState::Shift())
  {
    int nID = 0;
    int n = e.GetId();
    if(n == IDgraphLabelsCombo)
    {
      nID = IDmenuPlotLabels_BEGIN + e.GetSelection();
    }
    else if(n == IDgraphArtifactCombo)
    {
      nID = IDmenuPlotArtifactsNone + e.GetSelection();
    }
    if(nID)
    {
      m_pFramePlot->SyncState(this,nID);
    }
  }
  UpdateGridLabels();
}

void CPanelPlot::UpdateGridLabels(int nLabel)
{
  if(IsPreview())
  {
    // unless "none" is selected, update grid
    if(nLabel < 0)
    {
      nLabel = m_pMenu->LabelType();
    }
    if(nLabel)
    {
      m_pFrameAnalysis->SetLabelType(PLOT_TO_CELL(nLabel));
    }
  }
}
void CPanelPlot::SetSashAndMinHeight(bool bShowSash, int nHeight)
{
  if(!bShowSash)
  {
    nHeight = -1;
  }
  wxSize sizeCurrent = GetSize();
  wxSize sz(-1,nHeight);
  SetMinSize(sz);
  if(sizeCurrent.GetHeight() < nHeight)
  {
    sizeCurrent.SetHeight(nHeight);
    SetSize(sizeCurrent);
  }
  SetSashVisible(wxSASH_BOTTOM, bShowSash);
}
bool CPanelPlot::CanShowPeakArea()
{
  bool bRtn = (m_pOARfile != NULL);
  if(bRtn)
  {
    COARsample *pSample =
      m_pOARfile->GetSampleByName(m_pData->GetFilename());
    bRtn = (pSample != NULL) && !pSample->IsLadderType();
  }
  return bRtn;
}
void CPanelPlot::SetLabelType(LABEL_PLOT_TYPE n,LABEL_PLOT_TYPE nDefault)
{
  m_pMenu->SetLabelType(n,nDefault);
  _SyncControllers(m_pMenu);
  LabelTypeChanged();
}
void CPanelPlot::_SetArtifactLabel(int n)
{
  if(n != m_pMenu->ArtifactValue())
  {
    m_pMenu->SetArtifactValue(n);
    _SyncControllers(m_pMenu);
    LabelTypeChanged();
  }
}
void CPanelPlot::BuildMinRfuLines()
{
  if(!m_mapMinRfuAll.size())
  {
    set<double> setD;
    double d;
    double X[2] = {0.0, m_pData->GetMaxTime()};
    double Y[2] = {0.0,0.0};
    double *pdx;
    double *pdy;
    unsigned int nChannels = m_pData->GetChannelCount();
    unsigned int i;
    for(i = 1; i <= nChannels; i++)
    {
      d = m_pData->GetMinRfu(i);
      if(d > 0.0)
      {
        setD.insert(d);
      }
    }
    for(set<double>::iterator itr = setD.begin();
      itr != setD.end();
      ++itr)
    {
      d = *itr;
      Y[0] = d;
      Y[1] = d;
      pdx = Copy2Points(X);
      pdy = Copy2Points(Y);
      wxPlotData *pPlotData =
        new wxPlotData(pdx,pdy,2,false);
      m_mapMinRfuAll.insert(mapMinRfu::value_type(d,pPlotData));
    }
  }
}
wxColour CPanelPlot::COLOUR_RFU(64,64,64);
wxColour CPanelPlot::GetMinRfuColour()
{
  return COLOUR_RFU;
}
void CPanelPlot::BuildRfuSet(set<double> *psetD)
{
  double d;
  psetD->clear();
  unsigned int nChannels = m_pData->GetChannelCount();
  for(unsigned int i = 1; i <= nChannels; i++)
  {
    if(m_pMenu->ChannelValue(i))
    {
      d = m_pData->GetMinRfu(i);
      if(d > 0.0)
      {
        psetD->insert(d);
      }
    }
  }

}
void CPanelPlot::ShowMinRfuLines()
{
  set<double> setD;
  mapMinRfu::iterator itrm;
  wxPlotData *pData;

  BuildMinRfuLines();
  BuildRfuSet(&setD);
  m_setMinRfu.clear();
  wxColour clr = GetMinRfuColour();
  wxGenericPen pen(clr);
  for(set<double>::iterator itr = setD.begin();
    itr != setD.end();
    ++itr)
  {
    itrm = m_mapMinRfuAll.find(*itr);
    if(itrm != m_mapMinRfuAll.end())
    {
      pData = itrm->second;
      pData->SetPen(wxPLOTPEN_NORMAL,pen);
      pData->SetPen(wxPLOTPEN_ACTIVE,pen);
      pData->SetPen(wxPLOTPEN_SELECTED,pen);
      m_pPlotCtrl->AddCurve(*pData,false,false);
      m_setMinRfu.insert(*itr);
    }
  }
}

void CPanelPlot::ExpandRect(wxRect2DDouble *p,double dBy)
{
  double d = p->m_height * dBy;;
  p->m_y -= d;
  p->m_height += (d + d);

  d = p->m_width * dBy;
  p->m_x -= d;
  p->m_width += (d + d);
}
void CPanelPlot::ExtendLabelHeight(wxRect2DDouble *p)
{
  double dExtend = 1.0;
  wxRect rect = m_pPlotCtrl->GetPlotAreaRect();
  if(rect.height > 16)
  {
    dExtend = double(rect.height) / double(rect.height - 8);
    p->m_height *= dExtend;
  }
}
wxRect2DDouble CPanelPlot::GetZoomOutRect(bool bAll)
{
  wxRect2DDouble rtn(0.0,0.0,1.0,1.0);
  int nStart = 0;
  int nCount = m_pPlotCtrl->GetCurveCount();
  if(m_pMenu->MinRfuValue())
  {
    nStart += (int) m_setMinRfu.size();
  }
  if(m_pMenu->ILSValue())
  {
    nStart += (int)m_vILS.size() ;
  }
  if(!bAll)
  {
    rtn.m_x = (double) m_pData->GetBegin();
    nStart += m_nNoiseCurves;
  }
  for(int i = nStart; i < nCount; i++)
  {
    rtn.Union(m_pPlotCtrl->GetCurve(i)->GetBoundingRect());
  }
  ExpandRect(&rtn);
  ExtendLabelHeight(&rtn);
  return rtn;
}
void CPanelPlot::ZoomToLocus(const wxString &sLocus, unsigned int nDelay)
{
  wxRect2DDouble r = GetZoomLocus(sLocus);
  SetViewRect(r,false,nDelay);
}
wxRect2DDouble CPanelPlot::GetZoomLocus(const wxString &sLocus)
{
  wxRect2DDouble rectRtn(0.0,0.0,1.0,1.0);
  int nx1;
  int nx2;
  int ny1;
  int ny2;
  int nType = GetPlotTypes();
  if( (!sLocus.IsEmpty()) && m_pData->GetLocusRange(sLocus,&nx1,&nx2,&ny1,&ny2,nType) )
  {
    CParmOsirisGlobal parm;
    double dADJUST(parm->GetZoomLocusMargin());
    double dx = (double) nx1;
    double dy = (double) ny1;
    double dh = (double) (ny2 - ny1);
    double dw = (double) (nx2 - nx1);
    if(dADJUST > 0.0)
    {
      if(dADJUST >= 1.0)
      {
        dADJUST *= 0.01;
      }
      if(dADJUST > 0.5)
      {
        dADJUST = 0.5;
      }
      double dAdjustX = dw * dADJUST;
      dx -= dAdjustX;
      dw += (dAdjustX + dAdjustX);
      if(dx < 0.0)
      {
        dw += dx;
        dx = 0.0;
      }
    }
    rectRtn = wxRect2DDouble(dx,dy,dw,dh);
    ExpandRect(&rectRtn);
    ExtendLabelHeight(&rectRtn);
  }
  else
  {
    rectRtn = GetZoomOutRect();
  }
  return rectRtn;
}

void CPanelPlot::ShowILSlines()
{
  if(m_vILS.empty())
  {
    BuildILSlines();
  }
  m_nILScurveOffset = m_pPlotCtrl->GetCurveCount();
  for(vectorILSlines::iterator itr = m_vILS.begin();
    itr != m_vILS.end();
    ++itr)
  {
    wxPlotData *p = *itr;
    m_pPlotCtrl->AddCurve(*p,false,false);
  }
}

void CPanelPlot::ShowAllChannels(bool bShow)
{
  set<int> setCh;
  m_pData->GetChannelNumbers(&setCh);
  for(set<int>::iterator itr = setCh.begin();
    itr != setCh.end();
    ++itr)
  {
    ShowChannel((unsigned int)(*itr),bShow);
  }
}
void CPanelPlot::SetupLabelMenus()
{
  bool b = CanShowPeakArea();
  m_pMenu->EnablePeakAreaLabel(b);
  if(m_pButtonPanel != NULL)
  {
    m_pButtonPanel->EnablePeakAreaLabel(b);
  }
}
void CPanelPlot::SetPlotSettings()
{
  CParmOsirisGlobal parm;
  bool bPeak = CanShowPeakArea();
  bool bAnalyzed = parm->GetPlotDataAnalyzed();
  bool bRaw = parm->GetPlotDataRaw();
  bool bLadder = parm->GetPlotDataLadder();
  bool bBaseline = parm->GetPlotDataBaseline();
  bool bILS = parm->GetPlotShowILS();
  bool bRFU = parm->GetPlotShowRFU();
  bool bLadderLabels = parm->GetPlotShowLadderLabels();
  int nArt = (int)parm->GetPlotShowArtifact();
  int nLabel = (int)parm->GetPlotDisplayPeak();
  if(!(bRaw || bLadder || bAnalyzed))
  {
    bAnalyzed = true; // must show at least one
  }
  m_pMenu->EnablePeakAreaLabel(bPeak);
  m_pMenu->ShowAnalyzed(bAnalyzed);
  m_pMenu->ShowRaw(bRaw);
  m_pMenu->ShowLadder(bLadder);
  m_pMenu->ShowBaseline(bBaseline);
  m_pMenu->ShowILS(bILS);
  m_pMenu->ShowMinRfu(bRFU);
  m_pMenu->ShowLadderLabels(bLadderLabels);
  m_pMenu->SetLabelType((LABEL_PLOT_TYPE)nLabel,LABEL_ALLELE);
  m_pMenu->SetArtifactValue(nArt);
  _SyncControllers(m_pMenu);
}
void CPanelPlot::SetPreviewSettings()
{
  CParmOsirisGlobal parm;
  bool bPeak = CanShowPeakArea();
  bool bAnalyzed = parm->GetPreviewDataAnalyzed();
  bool bRaw = parm->GetPreviewDataRaw();
  bool bLadder = parm->GetPreviewDataLadder();
  bool bBaseline = parm->GetPreviewDataBaseline();
  bool bILS = parm->GetPreviewShowILS();
  bool bRFU = parm->GetPreviewShowRFU();
  bool bLadderLabels = parm->GetPreviewShowLadderLabels();
  int nArt = (int)parm->GetPreviewShowArtifact();
  int nLabel = parm->GetTableDisplayPeak();
  nLabel = CELL_TO_PLOT(nLabel);
  if(!(bRaw || bLadder || bAnalyzed))
  {
    bAnalyzed = true; // must show at least one
  }
  m_pMenu->EnablePeakAreaLabel(bPeak);
  m_pMenu->ShowAnalyzed(bAnalyzed);
  m_pMenu->ShowRaw(bRaw);
  m_pMenu->ShowLadder(bLadder);
  m_pMenu->ShowBaseline(bBaseline);
  m_pMenu->ShowILS(bILS);
  m_pMenu->ShowMinRfu(bRFU);
  m_pMenu->ShowLadderLabels(bLadderLabels);
  m_pMenu->SetLabelType((LABEL_PLOT_TYPE)nLabel,LABEL_NONE);
  m_pMenu->SetArtifactValue(nArt);
  _SyncControllers(m_pMenu);
}
void CPanelPlot::UpdateSettingsPlot()
{
  bool bAnalyzed = m_pMenu->AnalyzedValue();
  bool bRaw = m_pMenu->RawValue();
  bool bLadder = m_pMenu->LadderValue();
  bool bBaseline = m_pMenu->BaselineValue();
  bool bILS = m_pMenu->ILSValue();
  bool bRFU = m_pMenu->MinRfuValue();
  bool bLadderLabels = m_pMenu->LadderLabels();
  int nLabel = m_pMenu->LabelType();
  int nArt = m_pMenu->ArtifactValue();
  CParmOsirisGlobal parm;
  parm->SetPlotDataAnalyzed(bAnalyzed);
  parm->SetPlotDataLadder(bLadder);
  parm->SetPlotDataRaw(bRaw);
  parm->SetPlotDataBaseline(bBaseline);
  parm->SetPlotShowILS(bILS);
  parm->SetPlotShowRFU(bRFU);
  parm->SetPlotShowLadderLabels(bLadderLabels);
  if(!m_pMenu->UsingDefault())
  {
    parm->SetPlotDisplayPeak((unsigned int)nLabel);
  }
  parm->SetPlotShowArtifact((unsigned int)nArt);
}
void CPanelPlot::UpdateSettingsPreview()
{
  bool bAnalyzed = m_pMenu->AnalyzedValue();
  bool bRaw = m_pMenu->RawValue();
  bool bLadder = m_pMenu->LadderValue();
  bool bBaseline = m_pMenu->BaselineValue();
  bool bILS = m_pMenu->ILSValue();
  bool bRFU = m_pMenu->MinRfuValue();
  bool bLadderLabels = m_pMenu->LadderLabels();
  int nLabel = m_pMenu->LabelType();
  int nArt = m_pMenu->ArtifactValue();
  CParmOsirisGlobal parm;
  parm->SetPreviewDataAnalyzed(bAnalyzed);
  parm->SetPreviewDataLadder(bLadder);
  parm->SetPreviewDataBaseline(bBaseline);
  parm->SetPreviewDataRaw(bRaw);
  parm->SetPreviewShowILS(bILS);
  parm->SetPreviewShowRFU(bRFU);
  parm->SetPreviewShowLadderLabels(bLadderLabels);
  if(nLabel != LABEL_NONE)
  {
    int nPeak = PLOT_TO_CELL(m_pMenu->LabelType());
    parm->SetTableDisplayPeak(nPeak);
  }
  parm->SetPreviewShowArtifact((unsigned int)nArt);
}
void CPanelPlot::RebuildCurves(bool bIgnoreViewRect)
{
  unsigned int n;
  unsigned int anChannel[CHANNEL_MAX];
  unsigned int nChannel;
  unsigned int nChannelCount = m_pData->GetChannelCount();
  DATA_TYPE anData[COUNT_DATA];
  int ndxData = 0;
  int ndxChn = 0;
  int i;
  int j;
  m_nNoiseCurves = 0;
  wxRect2DDouble rect = GetViewRect();
  for(n = nChannelCount; n > 0; n--)
  {
    if(m_pMenu->ChannelValue(n))
    {
      anChannel[ndxChn++] = n;
    }
  }
  if(m_pMenu->LadderValue())
  {
    anData[ndxData++] = LADDER_DATA;
  }
  if(m_pMenu->RawValue())
  {
    anData[ndxData++] = RAW_DATA;
  }
  if(m_pMenu->AnalyzedValue())
  {
    anData[ndxData++] = ANALYZED_DATA;
  }
  if(m_pMenu->BaselineValue() && m_pData->HasBaseline())
  {
    anData[ndxData++] = BASELINE_DATA;
  }
  TnwxBatch<CPanelPlot> x(this);
  m_pPlotCtrl->DeleteCurve(-1);
  if(m_pMenu->MinRfuValue())
  {
    ShowMinRfuLines();
  }
  if(m_pMenu->ILSValue())
  {
    ShowILSlines();
  }
  bool bNoise = true;
  for(int zz = 0; zz < 2; zz++)
  {
    for(i = 0; i < ndxChn; i++)
    {
      nChannel = anChannel[i];
      for(j = 0; j < ndxData; j++)
      {
        wxPlotData *p(FindData(anData[j],nChannel,bNoise));
        if(p != NULL)
        {
          m_pPlotCtrl->AddCurve(*p,false,false);
          if(bNoise)
          {
            m_nNoiseCurves++;
          }
        }
      }
    }
    bNoise = false;
  }
  if(!bIgnoreViewRect)
  {
    SetViewRect(rect,false,0);
  }
  RebuildLabels();
  if(m_pMenu->SyncValue() && (m_pFramePlot != NULL))
  {
    m_pFramePlot->SyncTo(this);
  }
  Refresh();
}


wxMenuItem *CPanelPlot::GetMenuItem(wxMenu *p)
{
  nwxMenuItem *pMenuItem = m_pMenuItem.get();
  if(pMenuItem == NULL)
  {
    pMenuItem = new nwxMenuItem(
      p,                    // wxMenu *parent
      m_nMenuOffset,        // id
      _T("Plot Menu"),      // text
      _T(""),               // help
      wxITEM_NORMAL,        // wxItemKind
      m_pMenu);             // wxMenu *subMenu
    auto_ptr<nwxMenuItem> x(pMenuItem);
    m_pMenuItem = x;
  }
  else
  {
    pMenuItem->SetMenu(p);
  }
  return pMenuItem;
}
void CPanelPlot::_BuildMenu(int nMenuNr)
{
  if(m_pMenu == NULL)
  {
    m_nMenuOffset = ID_GET_PLOT_BASE(nMenuNr);
    if(!IsPreview())
    {
      // this is a grphics frame, build full menu
      // with offset
      m_pMenu = new CMenuPlot(m_nMenuOffset,m_pData, m_pColors);
      SetPlotSettings();
      m_pMenu->SetSync();
    }
    else
    {
      // this is an analysis frame with a graphic preview
      m_pMenu = new CMenuPlot(m_pData,m_pColors);
      SetPreviewSettings();
    }
    bool bPeak = CanShowPeakArea();
    m_pMenu->EnablePeakAreaLabel(bPeak);
  }
}

void CPanelPlot::ShowToolbar(bool bShow)
{
  if( HasToolbar() && (bShow != IsToolbarShown()) )
  {
//    m_pButtonPanel->Show(bShow);
    m_pShiftSizer->Show(bShow);
    if(bShow)
    {
      _SendSizeAction();
    }
  }
}
void CPanelPlot::CopySettings(CPanelPlot &w)
{
  TnwxBatch<CPanelPlot> x(this);
  TnwxBatch<CPanelPlot> y(&w);
  _SyncControllers(w.m_pMenu);
  wxRect2DDouble rect = w.GetViewRect();
  SetViewRect(rect,false,0);
  SyncToolbar(&w);
  RebuildCurves();
}
bool CPanelPlot::MenuEvent(wxCommandEvent &e)
{
  int nID = e.GetId() - m_nMenuOffset;
  bool bRebuild = false;
  bool bLabels = false;
  bool bRtn = true;
  bool bShift = (m_pFramePlot != NULL) && nwxKeyState::Shift();
  bool bSendToAll = false;
  if( !ID_PLOT_IS_PLOT(nID) )
  {
    wxString s;
    int nCurve = ID_GET_PLOT_NUMBER(m_nMenuOffset);
    s.Printf(
      _T("CPanelPlot::MenuEvent() id, "
        "%d, is out of range, [%d,%d], curve # %d"),
      nID,IDmenuPlotBase + 1, IDmenuPlot_MAX - 1,nCurve);
    wxASSERT_MSG(0,s);
    mainApp::LogMessage(s);
  }
  else if(ID_PLOT_IS_LOCUS(nID))
  {
    // zoom to a locus
  }
  else if(ID_PLOT_IS_LABELS(nID))
  {
    // set display type
    bSendToAll = bShift;
    bLabels = true;
    if(IsPreview())
    {
      UpdateGridLabels(nID - IDmenuPlotLabels_BEGIN);
    }
  }
  else
  {
    switch(nID)
    {
    case IDmenuPlotDataAnalyzed:
    case IDmenuPlotDataRaw:
    case IDmenuPlotDataLadder:
    case IDmenuPlotDataBaseline:
    case IDmenuPlotILS:
    case IDmenuPlotRFU:
    case IDmenuPlotLadderLabels:
      bSendToAll = bShift;
      bRebuild = true;
      break;
    case IDmenuPlotChannel1:
    case IDmenuPlotChannel2:
    case IDmenuPlotChannel3:
    case IDmenuPlotChannel4:
    case IDmenuPlotChannel5:
    case IDmenuPlotChannel6:
    case IDmenuPlotChannel7:
    case IDmenuPlotChannel8:
      if(nwxKeyState::Shift())
      {
        ShowOneChannel(nID - IDmenuPlotChannel1 + 1);
      }
      bRebuild = true;
      break;

    case IDmenuPlotSync:
      OnSync(e);
      break;

    case IDmenuPlotArtifactsNone:
    case IDmenuPlotArtifactsAll:
    case IDmenuPlotArtifactsCritical:
      bSendToAll = bShift;
      bLabels = true;
      break;

    case IDmenuPlotResetAxes:
    case IDmenuPlotResetAxesFull:
      OnZoomOut(e);
      break;

    case IDmenuPlotMultiple:
      OnBtnMultiple(e);
      break;

    case IDmenuPlotAppend:
      OnBtnAppend(e);
      break;

    case IDmenuPlotRemove:
    case IDmenuPlotRemoveOthers:
      OnBtnRemove(e);
      break;

    default:
      // this should never happen
      {
        wxString s;
        s.Printf("CPanelPlot::MenuEvent Invalid ID, %d", nID);
        wxASSERT_MSG(0,s);
        mainApp::LogMessage(s);
        bRtn = false;
      }
      break;
    }
  }
  if(bRebuild || bLabels)
  {
    CMenuPlot *pMenu = _GetLastMenu(); 
    _SyncControllers(pMenu);
    if(bRebuild)
    {
      RebuildCurves();
    }
    if(bLabels)
    {
      LabelTypeChanged();
    }
  }
  if(bSendToAll)
  {
    m_pFramePlot->SyncState(this,nID);
  }
  UpdateSettings();
  return bRtn;
}

CMDIFrame *CPanelPlot::_GetFrame()
{
  CMDIFrame *pRtn = 
    (m_pFramePlot == NULL)
    ? (CMDIFrame *)m_pFrameAnalysis
    : (CMDIFrame *)m_pFramePlot;
  return pRtn;
}
CMenuPlot *CPanelPlot::_GetLastMenu()
{
#if !REUSE_MENUS
  wxMenu *pLast = _GetFrame()->GetLastMenuShown();
  CMenuPlot *pMenu = 
    ( (pLast == (wxMenu *)m_pMenuPopup) && (pLast != NULL) )
    ? m_pMenuPopup
    : m_pMenu;
  return pMenu;
#else
  return m_pMenu;
#endif
}
void CPanelPlot::_SyncControllers(CPlotController *pSyncTo)
{
  CPlotController *pControl[] =
  {
#if !REUSE_MENUS
    m_pMenuPopup,
#endif
    m_pMenu,
    m_pButtonPanel
  };
  size_t N = sizeof(pControl) / sizeof(pControl[0]);
  CPlotController **pp = pControl;
  for(size_t i = 0; i < N; i++)
  {
    if((*pp != NULL) && (*pp != pSyncTo))
    {
      (*pp)->CopySettings(*pSyncTo);
    }
    pp++;
  }
}
void CPanelPlot::SyncState(CPanelPlot *p, int nID)
{
  //
  //  copy the state of a particular 
  //  setting to be identical to CPanelPlot *p
  //
  bool bRebuild = false;
  bool bLabels = false;
  CMenuPlot *pMenu = _GetLastMenu();

  if(p == this)
  {
    ; // do nothing
  }
  else if(ID_PLOT_IS_LABELS(nID))
  {
    bLabels = true;
    SetLabelType(p->m_pMenu->LabelType());
  }
  else
  {
    switch(nID)
    {
    case IDmenuPlotDataAnalyzed:
      pMenu->ShowAnalyzed(p->m_pMenu->AnalyzedValue());
      bRebuild = true;
      break;
    case IDmenuPlotDataRaw:
      pMenu->ShowRaw(p->m_pMenu->RawValue());
      bRebuild = true;
      break;
    case IDmenuPlotDataLadder:
      pMenu->ShowLadder(p->m_pMenu->LadderValue());
      bRebuild = true;
      break;
    case IDmenuPlotDataBaseline:
      pMenu->ShowBaseline(p->m_pMenu->BaselineValue());
      bRebuild = true;
      break;
    case IDmenuPlotILS:
      pMenu->ShowILS(p->m_pMenu->ILSValue());
      bRebuild = true;
      break;
    case IDmenuPlotRFU:
      pMenu->ShowMinRfu(p->m_pMenu->MinRfuValue());
      bRebuild = true;
      break;
    case IDmenuPlotLadderLabels:
      pMenu->ShowLadderLabels(p->m_pMenu->LadderLabels());
      bRebuild = true;
      break;
    case IDmenuPlotSync:
      pMenu->SetSync(p->m_pMenu->SyncValue());
      bRebuild = true;
      break;

    case IDmenuPlotArtifactsNone:
    case IDmenuPlotArtifactsAll:
    case IDmenuPlotArtifactsCritical:
      pMenu->SetArtifactValue(p->m_pMenu->ArtifactValue());
      bLabels = true;
      break;

    }
  }
  if(bRebuild || bLabels)
  {
    _SyncControllers(pMenu);
    if(bRebuild)
    {
      RebuildCurves();
    }
    if(bLabels)
    {
      LabelTypeChanged();
    }
  }
}

void CPanelPlot::OnContextMenu(wxContextMenuEvent &e)
{
  wxPoint pt = e.GetPosition();
  if((pt.x >= 0) && (pt.y >= 0) && (m_pMenu != NULL))
  {
#if !REUSE_MENUS
    if(m_pMenuPopup == NULL)
    {
      if(IsPreview())
      {
        m_pMenuPopup = new CMenuPlot(m_pData,m_pColors);
      }
      else
      {
        m_pMenuPopup = new CMenuPlot(m_nMenuOffset,m_pData,m_pColors);
      }
      m_pMenuPopup->CopySettings(*m_pMenu);
    }
    _GetFrame()->PopupMenu_(m_pMenuPopup);
#else
    _GetFrame()->PopupMenu_(m_pMenu);
#endif
  }
  else
  {
    e.Skip();
  }
}
void CPanelPlot::OnSizeAction(wxCommandEvent &)
{
  if(IsToolbarShown() && (m_pShiftSizer != NULL))
  {
    m_pShiftSizer->UpdateSize();
  }
}
void CPanelPlot::OnCommandEnter(wxCommandEvent &e)
{
  if(e.GetId() != wxID_ANY)
  {
    e.Skip(true);
  }
  else
  {
    OnSizeAction(e);
  }
}


void CPanelPlot::OnSize(wxSizeEvent &e)
{
  // when the window is maximized, the size is not reported
  // corectly, so add a pending event in order to delay action
  _SendSizeAction();
  e.Skip();
}

BEGIN_EVENT_TABLE(CPanelPlot,wxSashWindow)
EVT_COMBOBOX(IDgraphLabelsCombo, CPanelPlot::OnLabelTypeChanged)
EVT_COMBOBOX(IDgraphArtifactCombo, CPanelPlot::OnLabelTypeChanged)

EVT_TOGGLEBUTTON(IDgraphRebuild, CPanelPlot::OnRebuildCurves)

EVT_TOGGLEBUTTON(IDgraphSyncAxes, CPanelPlot::OnSync)
EVT_BUTTON(IDgraphZoomOut, CPanelPlot::OnZoomOut)
EVT_BUTTON(IDgraphAppend, CPanelPlot::OnBtnAppend)
EVT_BUTTON(IDgraphMultiple, CPanelPlot::OnBtnMultiple)
EVT_BUTTON(IDgraphRemove, CPanelPlot::OnBtnRemove)
EVT_BUTTON(IDbuttonDetails, CPanelPlot::OnBtnDetails)
EVT_PLOTCTRL_VIEW_CHANGED(wxID_ANY, CPanelPlot::OnViewChanged)

EVT_PLOTCTRL_POINT_DOUBLECLICKED(wxID_ANY,CPanelPlot::OnPointSelected)
EVT_PLOTCTRL_POINT_CLICKED(wxID_ANY,CPanelPlot::OnPointSelected)

EVT_TIMER(IDtimer,CPanelPlot::OnTimerEvent)
EVT_CONTEXT_MENU(CPanelPlot::OnContextMenu)
EVT_COMMAND_ENTER(wxID_ANY,CPanelPlot::OnCommandEnter)

EVT_SIZE(CPanelPlot::OnSize)
END_EVENT_TABLE()




void CPanelPlotToolbarSaveState::Setup(CPanelPlot *pWin, bool bRestoreCurrent)
{
  if(bRestoreCurrent)
  {
    RestoreState(true);
  }
  if(pWin != NULL)
  {
    m_pToolbar = pWin->GetToolbar();
  }
  else
  {
    m_pToolbar = NULL;
  }
  if(m_pToolbar != NULL)
  {
    int nY;
    m_pWin = pWin;
    m_pToolbar->GetPosition(&m_nX,&nY);
    m_bCanShiftLeft = m_pToolbar->CanShiftLeft();
    m_bCanShiftRight = m_pToolbar->CanShiftRight();
  }
  else
  {
    m_pWin = NULL;
  }
}

CPanelPlotToolbarSaveState::~CPanelPlotToolbarSaveState()
{
  RestoreState();
}
CPanelPlot *CPanelPlotToolbarSaveState::RestoreState(bool bRemove)
{
  CPanelPlot *pRtn = m_pWin;
  if(m_pToolbar == NULL)
  {} // done
  else if(!m_bCanShiftLeft)
  {
    m_pToolbar->ShiftLeft(true);
  }
  else if(!m_bCanShiftRight)
  {
    m_pToolbar->ShiftRight(true);
  }
  else
  {
    wxSize szV = m_pToolbar->GetVirtualSize();
    wxSize szC = m_pToolbar->GetClientSize();
    int nMinX = szC.GetWidth() - szV.GetWidth();
    if(m_nX < nMinX)
    {
      m_nX = nMinX;
    }
    m_pToolbar->Move(m_nX,0);
  }
  if(bRemove)
  {
    m_pToolbar = NULL;
    m_pWin = NULL;
  }
  return pRtn;
}
