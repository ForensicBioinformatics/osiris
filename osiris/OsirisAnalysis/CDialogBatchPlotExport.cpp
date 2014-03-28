#include "CDialogBatchPlotExport.h"
CDialogBatchPlotExport::CDialogBatchPlotExport(wxWindow *parent) : wxDialog(parent, wxID_ANY, _T("Batch Plot Export"), wxDefaultPosition, parent->GetSize()) {
	CParmOsirisGlobal parm;
	parm->GetGlobal();
	wxPanel *panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, this->GetSize());
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	topsizer->AddSpacer(10);
		wxBoxSizer *zoomGroupSizer = new wxBoxSizer(wxHORIZONTAL);
		zoomGroupSizer->AddSpacer(20);
			wxBoxSizer *zoomSizer = new wxBoxSizer(wxVERTICAL);
			zoomChoiceArray = new wxArrayString();
			zoomChoiceArray->Add(_T("Default"));
			zoomChoiceArray->Add(_T("Custom"));
			zoom = new wxRadioBox(panel, IDzoomOptions, _T("Plot Zoom"), wxDefaultPosition, wxDefaultSize, *zoomChoiceArray, 1);
			zoom->SetSelection(parm->GetBatchZoom());
			zoomSizer->Add(zoom, 0, wxALIGN_CENTER);
		zoomGroupSizer->Add(zoomSizer, 0, wxALIGN_CENTER);
		zoomGroupSizer->AddSpacer(10);
			wxStaticBoxSizer *zoomInputSizer = new wxStaticBoxSizer(wxHORIZONTAL, panel, wxT("Plot Parameters"));
				wxBoxSizer *minSizer = new wxBoxSizer(wxVERTICAL);
					wxBoxSizer *minTimeSizer = new wxBoxSizer(wxHORIZONTAL);
					minTimeLabel = new wxStaticText(panel, wxID_ANY, wxT("Min Time:"));
					minTimeSizer->Add(minTimeLabel, 0, wxALIGN_CENTER | wxRIGHT, 10);
					minTime = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(50, 15), wxTE_CENTER | wxTE_NOHIDESEL);
					minTime->SetMaxLength(10L);
					minTime->SetValue(wxString::Format(wxT("%i"), parm->GetBatchMinTime()));
					minTimeSizer->Add(minTime, 0, wxALIGN_CENTER);
				minSizer->Add(minTimeSizer, 1, wxEXPAND | wxALL, 1);
					wxBoxSizer *minRFUSizer = new wxBoxSizer(wxHORIZONTAL);
					minRFULabel = new wxStaticText(panel, wxID_ANY, wxT("Min RFU:"));
					minRFUSizer->Add(minRFULabel, 0, wxALIGN_CENTER | wxRIGHT, 12);
					minRFU = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(50, 15), wxTE_CENTER);
					minRFU->SetMaxLength(10L);
					minRFU->SetValue(wxString::Format(wxT("%i"), parm->GetBatchMinRFU()));
					minRFUSizer->Add(minRFU, 0, wxALIGN_CENTER);
				minSizer->Add(minRFUSizer, 1, wxEXPAND | wxALL, 1);
			zoomInputSizer->Add(minSizer, 1, wxCENTER | wxALL, 4);
				wxBoxSizer *maxSizer = new wxBoxSizer(wxVERTICAL);
					wxBoxSizer *maxTimeSizer = new wxBoxSizer(wxHORIZONTAL);
					maxTimeLabel = new wxStaticText(panel, wxID_ANY, wxT("Max Time:"));
					maxTimeSizer->Add(maxTimeLabel, 0, wxALIGN_CENTER | wxRIGHT, 10);
					maxTime = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(50, 15), wxTE_CENTER);
					maxTime->SetMaxLength(10L);
					maxTime->SetValue(wxString::Format(wxT("%i"), parm->GetBatchMaxTime()));
					maxTimeSizer->Add(maxTime, 0, wxALIGN_CENTER);
				maxSizer->Add(maxTimeSizer, 1, wxEXPAND | wxALL, 1);
					wxBoxSizer *maxRFUSizer = new wxBoxSizer(wxHORIZONTAL);
					maxRFULabel = new wxStaticText(panel, wxID_ANY, wxT("Max RFU:"));
					maxRFUSizer->Add(maxRFULabel, 0, wxALIGN_CENTER | wxRIGHT, 12);
					maxRFU = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(50, 15), wxTE_CENTER);
					maxRFU->SetMaxLength(10L);
					maxRFU->SetValue(wxString::Format(wxT("%i"), parm->GetBatchMaxRFU()));
					maxRFUSizer->Add(maxRFU, 0, wxALIGN_CENTER);
				maxSizer->Add(maxRFUSizer, 1, wxEXPAND | wxALL, 1);
			zoomInputSizer->AddSpacer(10);
			zoomInputSizer->Add(maxSizer, 1, wxCENTER | wxALL, 4);
		zoomGroupSizer->Add(zoomInputSizer, 0, wxALIGN_CENTER);
		zoomGroupSizer->AddSpacer(20);
	_SetZoomControls();
	topsizer->Add(zoomGroupSizer, 0, wxALIGN_CENTER);	
		wxBoxSizer *labelGroupSizer = new wxBoxSizer(wxHORIZONTAL);
			labelGroupSizer->AddSpacer(10);
			wxBoxSizer *styleSizer = new wxBoxSizer(wxVERTICAL);
			styleChoiceArray = new wxArrayString();
			styleChoiceArray->Add(_T("Multiple Lines"));
			styleChoiceArray->Add(_T("Single Line"));
			style = new wxRadioBox(panel, IDstyleMenu, _T("Label Style"), wxDefaultPosition, wxDefaultSize, *styleChoiceArray, 1, wxRA_SPECIFY_COLS);
			style->SetSelection(!parm->GetMultiLineLabel());
			styleSizer->Add(style, 1, wxEXPAND | wxALL, 10);
		labelGroupSizer->Add(styleSizer, 0, wxALIGN_CENTER);
			wxBoxSizer *checkSizer = new wxStaticBoxSizer(wxVERTICAL, panel, _T("Label Data"));
				wxBoxSizer *rowOneSizer = new wxBoxSizer(wxHORIZONTAL);
				alleles = new wxCheckBox(panel, wxID_ANY, _T("Allele"));
				alleles->SetValue(parm->GetMultiLabelAllele());
				rowOneSizer->Add(alleles, 1, wxEXPAND | wxALL, 1);
				bps = new wxCheckBox(panel, wxID_ANY, _T("BPS"));
				bps->SetValue(parm->GetMultiLabelBPS());
				rowOneSizer->Add(bps, 1, wxEXPAND | wxALL, 1);
				rfu = new wxCheckBox(panel, wxID_ANY, _T("RFU"));
				rfu->SetValue(parm->GetMultiLabelRFU());
				rowOneSizer->Add(rfu, 1, wxEXPAND | wxALL, 1);
			checkSizer->Add(rowOneSizer, 1, wxEXPAND | wxALL, 1);
				wxBoxSizer *rowTwoSizer = new wxBoxSizer(wxHORIZONTAL);
				time = new wxCheckBox(panel, wxID_ANY, _T("Time"));
				time->SetValue(parm->GetMultiLabelTime());
				rowTwoSizer->Add(time, 1, wxEXPAND | wxALL, 1);
				peak_area = new wxCheckBox(panel, wxID_ANY, _T("Peak Area"));
				peak_area->SetValue(parm->GetMultiLabelPeakArea());
				rowTwoSizer->Add(peak_area, 1, wxEXPAND | wxALL, 1);
				rowTwoSizer->AddStretchSpacer();
			checkSizer->Add(rowTwoSizer, 1, wxEXPAND | wxALL, 3);
		labelGroupSizer->Add(checkSizer, 1, wxALIGN_CENTER | wxALL, 1);
		labelGroupSizer->AddSpacer(20);
	topsizer->Add(labelGroupSizer, 1, wxALIGN_CENTER);
		wxStaticBoxSizer *previewSizer = new wxStaticBoxSizer(wxHORIZONTAL, panel, _T("Preview"));
		preview_box = new wxStaticText(panel, IDpreviewBox, GetPreviewString(!parm->GetMultiLineLabel()), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
		previewSizer->Add(preview_box, 1, wxEXPAND | wxALL, 10);
	topsizer->Add(previewSizer, 0, wxALIGN_CENTER);
		wxBoxSizer *inputSizer = new wxBoxSizer(wxHORIZONTAL);
		cancel = new wxButton(panel, wxID_CANCEL, _T("Cancel"));
		inputSizer->Add(cancel, 1, wxEXPAND | wxALL, 10);
		exportBtn = new wxButton(panel, wxID_OK, _T("Export"));
		inputSizer->Add(exportBtn, 1, wxEXPAND | wxALL, 10);
	topsizer->Add(inputSizer, 0, wxALIGN_CENTER);
	SetSizerAndFit(topsizer);
	CenterOnParent();
	ShowModal();
}
CDialogBatchPlotExport::~CDialogBatchPlotExport() {
	delete zoomChoiceArray;
	zoom->Destroy();
	minTimeLabel->Destroy();
	minTime->Destroy();
	minRFULabel->Destroy();
	minRFU->Destroy();
	maxTimeLabel->Destroy();
	maxTime->Destroy();
	maxRFULabel->Destroy();
	maxRFU->Destroy();
	delete styleChoiceArray;
	style->Destroy();
	alleles->Destroy();
	bps->Destroy();
	rfu->Destroy();
	time->Destroy();
	peak_area->Destroy();
	preview_box->Destroy();
	cancel->Destroy();
	exportBtn->Destroy();
}
bool CDialogBatchPlotExport::TransferDataFromWindow() {
	bool bRtn = true;
	CParmOsirisGlobal parm;
	parm->GetGlobal();
	parm->SetMultiLabelAllele(alleles->IsChecked());
	parm->SetMultiLabelBPS(bps->IsChecked());
	parm->SetMultiLabelRFU(rfu->IsChecked());
	parm->SetMultiLabelTime(time->IsChecked());
	parm->SetMultiLabelPeakArea(peak_area->IsChecked());
	if (style->GetSelection() == 1) parm->SetMultiLineLabel(false); else parm->SetMultiLineLabel(true);
	parm->SetBatchZoom(zoom->GetSelection());
	(minTime->GetValue().IsNumber()) ? parm->SetBatchMinTime(wxAtoi(minTime->GetValue())) : parm->SetBatchMinTime(0);
	(maxTime->GetValue().IsNumber()) ? parm->SetBatchMaxTime(wxAtoi(maxTime->GetValue())) : parm->SetBatchMaxTime(0);
	(minRFU->GetValue().IsNumber()) ? parm->SetBatchMinRFU(wxAtoi(minRFU->GetValue())) : parm->SetBatchMinRFU(0);
	(maxRFU->GetValue().IsNumber()) ? parm->SetBatchMaxRFU(wxAtoi(maxRFU->GetValue())) : parm->SetBatchMaxRFU(0);
	parm->SetAbortExport(false);
	return bRtn;
}
wxString CDialogBatchPlotExport::GetPreviewString(bool single) {
	wxString rStr = wxT("");
	vector<string> *selection = new vector<string>();
	selection->resize(5);
	if (peak_area->IsChecked()) {
		if (single) selection->push_back("<peak area>"); else selection->push_back("PA: <peak area>");
	}
	if (time->IsChecked()) {
		if (single) selection->push_back("<time>"); else selection->push_back("TI: <time>");
	}
	if (rfu->IsChecked()) {
		if (single) selection->push_back("<RFU>"); else selection->push_back("RF: <RFU>");
	}
	if (bps->IsChecked()) {
		if (single) selection->push_back("<BPS>"); else selection->push_back("BP: <BPS>");
	}
	if (alleles->IsChecked()) {
		if (single) selection->push_back("<allele>"); else selection->push_back("AL: <allele>");
	}
	selection->shrink_to_fit();
	for (unsigned int i = 0; i < selection->size(); i++) {
		rStr += wxT(selection->back());
		selection->pop_back();
		if (selection->back() != wxEmptyString) {
			if (single) rStr += wxT(", "); else rStr += wxT("\n");
		}
	}
	delete selection;
	return rStr;
}
void CDialogBatchPlotExport::OnChangeLabel(wxCommandEvent &) {
	parm->GetGlobal();
	bool b = (style->GetSelection() == 0) ? false : true;
	preview_box->SetLabel(GetPreviewString(b));	
	Fit();
	Layout();
}
void CDialogBatchPlotExport::OnChangeZoom(wxCommandEvent &) {
	_SetZoomControls();
}
void CDialogBatchPlotExport::_SetZoomControls() {
	parm->GetGlobal();
	switch (zoom->GetSelection()) {
	case 0:
		//default
		minTime->ChangeValue(wxT("0"));
		minTime->Disable();
		maxTime->ChangeValue(wxT("Auto"));
		maxTime->Disable();
		minRFU->ChangeValue(wxT("0"));
		minRFU->Disable();
		maxRFU->ChangeValue(wxT("Auto"));
		maxRFU->Disable();
		break;
	case 1:
		//custom
		if (parm->GetBatchMinTime() < 0) parm->SetBatchMinTime(0);
		if (parm->GetBatchMaxTime() < 0) parm->SetBatchMaxTime(0);
		if (parm->GetBatchMinRFU() < 0) parm->SetBatchMinRFU(0);
		if (parm->GetBatchMaxRFU() < 0) parm->SetBatchMaxRFU(0);
		minTime->ChangeValue(wxString::Format(wxT("%i"),parm->GetBatchMinTime()));
		minTime->Enable();
		maxTime->ChangeValue(wxString::Format(wxT("%i"), parm->GetBatchMaxTime()));
		maxTime->Enable();
		minRFU->ChangeValue(wxString::Format(wxT("%i"), parm->GetBatchMinRFU()));
		minRFU->Enable();
		maxRFU->ChangeValue(wxString::Format(wxT("%i"), parm->GetBatchMaxRFU()));
		maxRFU->Enable();
		break;
	default:
		break;
	}
}
BEGIN_EVENT_TABLE(CDialogBatchPlotExport, wxDialog)
EVT_CHECKBOX(wxID_ANY, CDialogBatchPlotExport::OnChangeLabel)
EVT_RADIOBOX(IDstyleMenu, CDialogBatchPlotExport::OnChangeLabel)
EVT_RADIOBOX(IDzoomOptions, CDialogBatchPlotExport::OnChangeZoom)
END_EVENT_TABLE()