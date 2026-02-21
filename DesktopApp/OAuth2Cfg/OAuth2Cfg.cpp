#include "OAuth2Cfg.h"
#include "../../CoreAppLib/AppDef.h"
#include "../UiHelper.h"

#define ProvChangeIndicator " *"

namespace OAuth2Cfg_Imp
{
	const wxChar* const Inf_ProvCreated = wxT("%i new provider(s) created");
	const wxChar* const Inf_ProvModified = wxT("%i provider(s) modified");
	const wxChar* const Inf_ProvDeleted = wxT("%i provider(s) removed");
	const wxChar* const Inf_Delimiter = wxT(", ");

	const wxChar* const Msg_ChangeSaveQuestion = wxT("Configuration has been changed:\n%s\n\nSave?");
	const wxChar* const Msg_ProvDeleteQuestion = wxT("Remove OAuth2 provider \"%s\"?");
	const wxChar* const Msg_ProvDataValidationError = wxT("Incorrect data for the provider \"%s\":\n%s");
	const wxChar* const Msg_ProvNameRequiredValues = wxT("provider must have a name");
	const wxChar* const Msg_ProvNameMustBeUnique = wxT("provider name must be unique");
	const wxChar* const Msg_GeneralSaveError = wxT("Something went wrong while applying the OAuth2 changes.");

	static void load_text_value(wxTextCtrl* ctrl, const std::string& value);
	static void load_prov_status(wxCheckBox* ctrl, const int& value);
	static bool save_text_value(std::string& value, const wxTextCtrl* ctrl);
	static bool save_prov_status(int& value, const wxCheckBox* ctrl);
}
using namespace OAuth2Cfg_Imp;

OAuth2Cfg::OAuth2Cfg(wxWindow* parent) : OAuth2CfgUI(parent)
{
	curSelProvIdx = -1;
	InitUI();
}

OAuth2Cfg::~OAuth2Cfg() { }

void OAuth2Cfg::InitUI()
{
	UiHelper::InitDialog(this);
	btnOk->Enable(false);
}

void OAuth2Cfg::LoadData()
{
	OAuth2EditMgr::GeneralSettings gen_cfg;
	std::vector<std::string> prov_names;
	edMgr.LoadData(gen_cfg, prov_names);
	chkVerifReq->SetValue(gen_cfg.VerificationRequired);
	for (size_t idx = 0; idx < prov_names.size(); ++idx) {
		lbxProviders->Append(
			wxString::FromUTF8(prov_names[idx]), new wxStringClientData(prov_names[idx]));
	}
}

void OAuth2Cfg::OAuth2CfgUI_OnInitDialog(wxInitDialogEvent& event)
{
	wxBeginBusyCursor();
	LoadData();
	wxEndBusyCursor();
}

OAuth2ProviderSettings* OAuth2Cfg::FindProvider(int sel_idx)
{
	if (sel_idx < 0) return nullptr;
	auto prov_name = lbxProviders->GetClientObject(sel_idx);
	return prov_name
		? edMgr.FindProvider(static_cast<wxStringClientData*>(prov_name)->GetData())
		: nullptr;
}

std::vector<wxString> OAuth2Cfg::ValidateProvData(const OAuth2ProviderSettings& prov)
{
	std::vector<wxString> errors;
	// Something to be validated after the item loaded
	return errors;
}

void OAuth2Cfg::LoadProvViewData(int sel_idx)
{
	curSelProvIdx = sel_idx;
	auto prov = FindProvider(sel_idx);
	bool is_new = nullptr == prov;
	if (is_new) prov = new OAuth2ProviderSettings;

	load_text_value(txtProvName, prov->Name);
	load_prov_status(chkProvStatus, prov->Status);
	load_text_value(txtAuthEndpoint, prov->AuthEndpoint);
	load_text_value(txtTokenEndpoint, prov->TokenEndpoint);
	load_text_value(txtScope, prov->Scope);
	load_text_value(txtClientId, prov->ClientId);
	load_text_value(txtClientSecret, prov->ClientSecret);
	load_text_value(txtComment, prov->Comment);

	if (is_new) delete prov;
	else {
		auto errors = ValidateProvData(*prov);
		if (!errors.empty()) ShowProvValidationErrors(prov, errors);
	}
	btnOk->Enable(true);
}

std::vector<wxString> OAuth2Cfg::ValidateProvViewData(const OAuth2ProviderSettings* prov)
{
	std::vector<wxString> errors;
	if (!prov) {
		errors.push_back(Msg_GeneralSaveError);
		return errors;
	}
	
	std::string prov_name;
	save_text_value(prov_name, txtProvName);
	if (prov_name.empty())
		errors.push_back(Msg_ProvNameRequiredValues);
	else if (edMgr.FindProvider(prov_name.c_str(), prov))
		errors.push_back(Msg_ProvNameMustBeUnique);

	return errors;
}

void OAuth2Cfg::ShowProvValidationErrors(const OAuth2ProviderSettings* prov, const std::vector<wxString>& errors)
{
	wxString error_list;
	for (const auto& item : errors) {
		error_list += " - " + item + "\n";
	}
	wxMessageBox(
		wxString::Format(Msg_ProvDataValidationError, wxString::FromUTF8(prov->Name), error_list),
		AppDef_Title, wxICON_ERROR | wxOK, this);
}

void OAuth2Cfg::toolProvCreate_OnToolClicked(wxCommandEvent& event)
{
	if (!CheckAndSaveProvViewData(curSelProvIdx)) return;
	std::string prov_name = edMgr.CreateProvider();
	// TODO: check the prov_name and show error if the value is empty (rare case, but better be handled)
	int sel_idx = lbxProviders->Append(prov_name + ProvChangeIndicator, new wxStringClientData(prov_name));
	lbxProviders->SetSelection(sel_idx);
	LoadProvViewData(sel_idx);
}

void OAuth2Cfg::toolProvDelete_OnToolClicked(wxCommandEvent& event)
{
	int sel_idx = lbxProviders->GetSelection();
	OAuth2ProviderSettings* prov = FindProvider(sel_idx);
	if (prov && wxOK == wxMessageBox(
		wxString::Format(Msg_ProvDeleteQuestion, wxString::FromUTF8(prov->Name)),
		AppDef_Title, wxICON_QUESTION | wxOK | wxCANCEL | wxCANCEL_DEFAULT, this))
	{
		edMgr.DeleteProvider(prov->Name.c_str());
		lbxProviders->Delete(sel_idx);
		lbxProviders->SetSelection(-1);
		LoadProvViewData(-1);
	}
}

void OAuth2Cfg::lbxProviders_OnListBox(wxCommandEvent& event)
{
	wxBeginBusyCursor();
	if (CheckAndSaveProvViewData(curSelProvIdx))
		LoadProvViewData(lbxProviders->GetSelection());
	else
		lbxProviders->SetSelection(curSelProvIdx);
	wxEndBusyCursor();
}

bool OAuth2Cfg::CheckAndSaveProvViewData(int sel_idx)
{
	if (sel_idx < 0) return true;
	auto prov = FindProvider(sel_idx);
	auto errors = ValidateProvViewData(prov);
	if (!errors.empty()) {
		ShowProvValidationErrors(prov, errors);
		return false;
	}

	bool is_changed = false;
	is_changed |= save_text_value(prov->Name, txtProvName);
	is_changed |= save_prov_status(prov->Status, chkProvStatus);
	is_changed |= save_text_value(prov->AuthEndpoint, txtAuthEndpoint);
	is_changed |= save_text_value(prov->TokenEndpoint, txtTokenEndpoint);
	is_changed |= save_text_value(prov->Scope, txtScope);
	is_changed |= save_text_value(prov->ClientId, txtClientId);
	is_changed |= save_text_value(prov->ClientSecret, txtClientSecret);
	is_changed |= save_text_value(prov->Comment, txtComment);

	if (is_changed && edMgr.SetProviderModified(prov->Name.c_str())) {
		static_cast<wxStringClientData*>(lbxProviders->GetClientObject(sel_idx))->
			SetData(prov->Name);
		lbxProviders->SetString(sel_idx, prov->Name + ProvChangeIndicator);
	}
	btnOk->Enable(true);
	return true;
}

bool OAuth2Cfg::GetChangeInfo(wxString& info)
{
	info.Clear();
	// TODO: indicate general settings changes as well, when they are available
	auto changes = edMgr.GetProvEditState();
	bool is_changed = changes.size() > 0;
	int state_value = changes[ListEditMgr_Def::EditState::Created];
	if (state_value > 0) info += wxString::Format(Inf_ProvCreated, state_value);
	state_value = changes[ListEditMgr_Def::EditState::Modified];
	if (state_value > 0) {
		if (!info.IsEmpty()) info += Inf_Delimiter;
		info += wxString::Format(Inf_ProvModified, state_value);
	}
	state_value = changes[ListEditMgr_Def::EditState::Deleted];
	if (state_value > 0) {
		if (!info.IsEmpty()) info += Inf_Delimiter;
		info += wxString::Format(Inf_ProvDeleted, state_value);
	}
	return is_changed;
}

void OAuth2Cfg::ApplyChanges()
{
	wxBeginBusyCursor();
	if (!edMgr.ApplyChanges())
		wxMessageBox(Msg_GeneralSaveError, AppDef_Title, wxICON_ERROR | wxOK, this);
	wxEndBusyCursor();
}

void OAuth2Cfg::btnOk_OnButtonClick(wxCommandEvent& event)
{
	if (!CheckAndSaveProvViewData(lbxProviders->GetSelection())) {
		return;
	}
	ApplyChanges();
	event.Skip();
}

void OAuth2Cfg::btnNo_OnButtonClick(wxCommandEvent& event)
{
	// Not saving any changes
	event.Skip();
}

void OAuth2Cfg::OAuth2CfgUI_OnClose(wxCloseEvent& event)
{
	if (!CheckAndSaveProvViewData(lbxProviders->GetSelection())) {
		event.Veto();
		return;
	}
	wxString change_info;
	bool is_changed = GetChangeInfo(change_info);
	if (is_changed && event.CanVeto())
		switch (wxMessageBox(wxString::Format(Msg_ChangeSaveQuestion, change_info), AppDef_Title,
			wxICON_QUESTION | wxYES | wxYES_DEFAULT | wxNO | wxCANCEL, this))
		{
		case wxYES: ApplyChanges(); break;
		case wxNO: // Not saving any changes
			break;
		case wxCANCEL: event.Veto(); return;
		}
	event.Skip();
}

// ***************************************** OAuth2Cfg_Imp *****************************************

static void OAuth2Cfg_Imp::load_text_value(wxTextCtrl* ctrl, const std::string& value)
{
	ctrl->SetValue(wxString::FromUTF8(value));
}

static void OAuth2Cfg_Imp::load_prov_status(wxCheckBox* ctrl, const int& value)
{
	ctrl->SetValue(OAuth2ProviderStatus_Enabled == value);
}

static bool OAuth2Cfg_Imp::save_text_value(std::string& value, const wxTextCtrl* ctrl)
{
	wxString txt_value(ctrl->GetValue().Trim());
	txt_value.Replace("\n", " ");
	txt_value.Replace("\r", "");
	wxCharBuffer new_value = txt_value.ToUTF8();
	if (value == new_value.data()) return false;
	value = new_value.data();
	return true;
}

static bool OAuth2Cfg_Imp::save_prov_status(int& value, const wxCheckBox* ctrl)
{
	int new_value = ctrl->IsChecked() ? OAuth2ProviderStatus_Enabled : OAuth2ProviderStatus_Disabled;
	if (value == new_value) return false;
	value = new_value;
	return true;
}
