#include "CDialogMultipleLabelSettings.h"
CDialogMultipleLabelSettings::CDialogMultipleLabelSettings(
	wxWindow *parent
	) : wxDialog(parent, wxID_ANY, _T("Multiple Label Settings"),wxDefaultPosition,parent->GetSize()) {
	CParmOsirisGlobal parm;
	parm->GetGlobal();
	wxPanel *panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, this->GetSize());
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *labelGroupSizer = new wxBoxSizer(wxHORIZONTAL);
			wxBoxSizer *styleSizer = new wxBoxSizer(wxVERTICAL);
			styleChoiceArray = new wxArrayString();
			styleChoiceArray->Add(_T("Multiple Lines"));
			styleChoiceArray->Add(_T("Single Line"));
			style = new wxRadioBox(panel, IDstyleMenu, _T("Label Style"), wxDefaultPosition, wxDefaultSize, *styleChoiceArray, 1, wxRA_SPECIFY_COLS);
			style->SetSelection(!parm->GetMultiLineLabel());
			styleSizer->Add(style, 1, wxEXPAND | wxALL, 10);
		labelGroupSizer->Add(styleSizer, 0, wxALIGN_CENTER);
			wxStaticBoxSizer *checkSizer = new wxStaticBoxSizer(wxVERTICAL, panel, _T("Label Data"));
			wxBoxSizer *rowOne = new wxBoxSizer(wxHORIZONTAL);
			alleles = new wxCheckBox(panel, wxID_ANY, _T("Allele"));
			alleles->SetValue(parm->GetMultiLabelAllele());
			rowOne->Add(alleles, 1, wxEXPAND | wxALL, 1);
			bps = new wxCheckBox(panel, wxID_ANY, _T("BPS"));
			bps->SetValue(parm->GetMultiLabelBPS());
			rowOne->Add(bps, 1, wxEXPAND | wxALL, 1);
			checkSizer->Add(rowOne, 1, wxEXPAND | wxALL, 1);
			rfu = new wxCheckBox(panel, wxID_ANY, _T("RFU"));
			rfu->SetValue(parm->GetMultiLabelRFU());
			rowOne->Add(rfu, 1, wxEXPAND | wxALL, 1);
			time = new wxCheckBox(panel, wxID_ANY, _T("Time"));
			time->SetValue(parm->GetMultiLabelTime());
			wxBoxSizer *rowTwo = new wxBoxSizer(wxHORIZONTAL);
			rowTwo->Add(time, 1, wxEXPAND | wxALL, 1);
			peak_area = new wxCheckBox(panel, wxID_ANY, _T("Peak Area"));
			peak_area->SetValue(parm->GetMultiLabelPeakArea());
			rowTwo->Add(peak_area, 1, wxEXPAND | wxALL, 1);
			rowTwo->AddStretchSpacer();
			checkSizer->Add(rowTwo, 1, wxEXPAND | wxALL, 1);
		labelGroupSizer->Add(checkSizer, 1, wxALIGN_CENTER | wxALL, 10);
	topsizer->Add(labelGroupSizer, 1, wxALIGN_CENTER | wxALL, 10);
		wxStaticBoxSizer *previewSizer = new wxStaticBoxSizer(wxHORIZONTAL, panel, _T("Preview"));
		preview_box = new wxStaticText(panel, IDpreviewBox, GetPreviewString(!parm->GetMultiLineLabel()), wxDefaultPosition, wxDefaultSize, wxTE_CENTER);
		previewSizer->Add(preview_box, 1, wxEXPAND | wxALL, 10);
	topsizer->Add(previewSizer, 0, wxALIGN_CENTER);
		wxBoxSizer *inputSizer = new wxBoxSizer(wxHORIZONTAL);
		cancel = new wxButton(panel, wxID_CANCEL, _T("Cancel"));
		inputSizer->Add(cancel, 1, wxEXPAND | wxALL, 10);
		apply = new wxButton(panel, wxID_OK, _T("Apply"));
		inputSizer->Add(apply, 1, wxEXPAND | wxALL, 10);
	topsizer->Add(inputSizer, 0, wxALIGN_CENTER);
	SetSizerAndFit(topsizer);
	CenterOnParent();
	ShowModal();
}
CDialogMultipleLabelSettings::~CDialogMultipleLabelSettings() {
	delete styleChoiceArray;
	style->Destroy();
	alleles->Destroy();
	bps->Destroy();
	rfu->Destroy();
	time->Destroy();
	peak_area->Destroy();
	preview_box->Destroy();
	cancel->Destroy();
	apply->Destroy();
}
bool CDialogMultipleLabelSettings::TransferDataFromWindow() {
	bool bRtn = true;
	parm->GetGlobal();
	parm->SetMultiLabelAllele(alleles->IsChecked());
	parm->SetMultiLabelBPS(bps->IsChecked());
	parm->SetMultiLabelRFU(rfu->IsChecked());
	parm->SetMultiLabelTime(time->IsChecked());
	parm->SetMultiLabelPeakArea(peak_area->IsChecked());
	if (style->GetSelection() == 1) parm->SetMultiLineLabel(false); else parm->SetMultiLineLabel(true);
	return bRtn;
}
wxString CDialogMultipleLabelSettings::GetPreviewString(bool single) {
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
void CDialogMultipleLabelSettings::OnChange(wxCommandEvent &) {
	bool b = (style->GetSelection() == 0) ? false : true;
	preview_box->SetLabel(GetPreviewString(b));
	Fit();
	Layout();
}
BEGIN_EVENT_TABLE(CDialogMultipleLabelSettings, wxDialog)
EVT_CHECKBOX(wxID_ANY, CDialogMultipleLabelSettings::OnChange)
EVT_RADIOBOX(wxID_ANY, CDialogMultipleLabelSettings::OnChange)
END_EVENT_TABLE()