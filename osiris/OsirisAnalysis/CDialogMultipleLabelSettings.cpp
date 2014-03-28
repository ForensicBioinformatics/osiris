#include "CDialogMultipleLabelSettings.h"
CDialogMultipleLabelSettings::CDialogMultipleLabelSettings(
	wxWindow *parent
	) : wxDialog(parent, wxID_ANY, _T("Multiple Label Settings"),
	wxDefaultPosition, wxDefaultSize,
	wxDEFAULT_DIALOG_STYLE) {
	CParmOsirisGlobal parm;
	parm->GetGlobal();
	alleles = new wxCheckBox(this, wxID_ANY, _T("Allele"));
	alleles->SetValue(parm->GetMultiLabelAllele());
	bps = new wxCheckBox(this, wxID_ANY, _T("BPS"));
	bps->SetValue(parm->GetMultiLabelBPS());
	rfu = new wxCheckBox(this, wxID_ANY, _T("RFU"));
	rfu->SetValue(parm->GetMultiLabelRFU());
	time = new wxCheckBox(this, wxID_ANY, _T("Time"));
	time->SetValue(parm->GetMultiLabelTime());
	peak_area = new wxCheckBox(this, wxID_ANY, _T("Peak Area"));
	peak_area->SetValue(parm->GetMultiLabelPeakArea());
	wxArrayString *styleChoiceArray = new wxArrayString();
	styleChoiceArray->Add(_T("Multiple Lines"));
	styleChoiceArray->Add(_T("Single Line"));
	style = new wxRadioBox(this, IDstyleMenu, _T("Style"), wxDefaultPosition, wxDefaultSize, *styleChoiceArray, 1, wxRA_SPECIFY_ROWS);
	style->SetSelection(!parm->GetMultiLineLabel());
	preview_box = new wxTextCtrl(this, IDpreviewBox, GetPreviewString(), wxDefaultPosition, wxSize(350,80), wxTE_READONLY | wxTE_MULTILINE | wxTE_NO_VSCROLL | wxTE_CENTER);
	wxPanel *panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, this->GetSize());
	topsizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *checkGroup = new wxBoxSizer(wxHORIZONTAL);
	checkGroup->Add(alleles,1,wxEXPAND | wxALL,1);
	checkGroup->Add(bps,1,wxEXPAND | wxALL,1);
	checkGroup->Add(rfu,1,wxEXPAND | wxALL,1);
	checkGroup->Add(time,1,wxEXPAND | wxALL,1);
	checkGroup->Add(peak_area,1,wxEXPAND | wxALL,1);
	topsizer->Add(checkGroup,1,wxALIGN_CENTER);
	wxBoxSizer *styleSizer = new wxBoxSizer(wxVERTICAL);
	styleSizer->Add(style,1,wxEXPAND | wxALL,10);
	topsizer->Add(styleSizer,0,wxALIGN_CENTER);
	previewSizer = new wxBoxSizer(wxVERTICAL);
	previewSizer->Add(preview_box,1,wxEXPAND | wxALL,10);
	topsizer->Add(previewSizer,0,wxALIGN_CENTER);
	wxBoxSizer *inputSizer = new wxBoxSizer(wxHORIZONTAL);
	inputSizer->Add(new wxButton(panel, wxID_CANCEL, _T("Cancel")),0,wxALL,10);
	inputSizer->Add(new wxButton(panel, wxID_OK, _T("Apply")),0,wxALL,10);
	topsizer->Add(inputSizer,0,wxALIGN_CENTER);
	SetSizerAndFit(topsizer);
	CenterOnParent();
	ShowModal();
}
CDialogMultipleLabelSettings::~CDialogMultipleLabelSettings() {}
bool CDialogMultipleLabelSettings::TransferDataFromWindow() {
	bool bRtn = true;
	CParmOsirisGlobal parm;
	parm->GetGlobal();
	parm->SetMultiLabelAllele(alleles->IsChecked());
	parm->SetMultiLabelBPS(bps->IsChecked());
	parm->SetMultiLabelRFU(rfu->IsChecked());
	parm->SetMultiLabelTime(time->IsChecked());
	parm->SetMultiLabelPeakArea(peak_area->IsChecked());
	if (style->GetSelection() == 1) parm->SetMultiLineLabel(false); else parm->SetMultiLineLabel(true);
	return bRtn;
}
wxString CDialogMultipleLabelSettings::GetPreviewString() {
	wxString previewString = wxT("");
	vector<string> *selection = new vector<string>();
	selection->resize(5);
	if (peak_area->IsChecked()) selection->push_back("PA: <peak area>");
	if (time->IsChecked()) selection->push_back("TI: <time>");
	if (rfu->IsChecked()) selection->push_back("RF: <RFU>");
	if (bps->IsChecked()) selection->push_back("BP: <BPS>");
	if (alleles->IsChecked()) selection->push_back("AL: <allele>");
	for (unsigned int i = 0; i < selection->size(); i++) {
		previewString += wxT(selection->back());
		if (style->GetSelection() == 0) {
			if (selection->back() != wxEmptyString) previewString += wxT("\n");
		} 
		else
		{
			if (selection->back() != wxEmptyString)	previewString += wxT("; ");
		}
		selection->pop_back();
	}
	delete selection;
	return previewString;
}
void CDialogMultipleLabelSettings::OnChange(wxCommandEvent &) {
	preview_box->ChangeValue(GetPreviewString());
}
BEGIN_EVENT_TABLE(CDialogMultipleLabelSettings, wxDialog)
EVT_CHECKBOX(wxID_ANY, CDialogMultipleLabelSettings::OnChange)
EVT_RADIOBOX(wxID_ANY, CDialogMultipleLabelSettings::OnChange)
END_EVENT_TABLE()