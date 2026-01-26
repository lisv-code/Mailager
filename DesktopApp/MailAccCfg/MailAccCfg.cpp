#include "MailAccCfg.h"
#include "../../CoreAppLib/AppDef.h"
#include "../UiHelper.h"
#include "ConnectionUiCtrlHelper.h"
using namespace ConnectionUiCtrlHelper;

#define NewAccNamePrefix "new account #"
#define AccChangeIndicator " *"

namespace MailAccCfg_Imp
{
	const wxChar* const Inf_AccCreated = wxT("%i new account(s) created");
	const wxChar* const Inf_AccModified = wxT("%i account(s) modified");
	const wxChar* const Inf_AccDeleted = wxT("%i account(s) deleted");
	const wxChar* const Inf_Delimiter = wxT(", ");

	const wxChar* const Msg_ChangeSaveQuestion = wxT("Configuration has been changed:\n%s\n\nSave?");
	const wxChar* const Msg_AccDeleteQuestion = wxT("Delete account #%i \"%s\"?");
	const wxChar* const Msg_AccDataValidationError = wxT("Incorrect data for the account #%i \"%s\":\n%s");
	const wxChar* const Msg_AccDataRequiredValues = wxT("account must have a name or e-mail address or user name");
	const wxChar* const Msg_AccDataPortValueInvalid = wxT("port value must be a number between 1 and 65535");
	const wxChar* const Msg_GeneralSaveError = wxT("Something went wrong while applying the account changes.");
}
using namespace MailAccCfg_Imp;

MailAccCfg::MailAccCfg(wxWindow* Parent) : MailAccCfgUI(Parent)
{
	curSelAccIdx = -1;
	InitUI();
}

MailAccCfg::~MailAccCfg() { }

void MailAccCfg::InitUI()
{
	UiHelper::InitDialog(this);
	btnOk->Enable(false);

	init_prot_items(chcIncProto, true);
	init_prot_items(chcOutProto, false);
	init_auth_items(chcIncAuth);
	init_auth_items(chcOutAuth);
}

void MailAccCfg::LoadAccounts()
{
	auto accounts = accMgr.LoadAccounts();
	for (auto acc_inf : accounts) {
		chcAccount->Append(wxString::FromUTF8(acc_inf.second), (void*)acc_inf.first);
	}
}

AccountSettings* MailAccCfg::FindAccount(int sel_idx)
{
	if (sel_idx < 0) return nullptr;
	int acc_id = (int)chcAccount->GetClientData(sel_idx);
	return accMgr.FindAccount(acc_id);
}

void MailAccCfg::LoadViewData(int sel_idx)
{
	curSelAccIdx = sel_idx;
	auto acc = FindAccount(sel_idx);
	bool is_new = nullptr == acc;
	if (is_new) acc = new AccountSettings;

	txtAccName->SetValue(wxString::FromUTF8(acc->AccountName.c_str()));
	txtEmailAddr->SetValue(wxString::FromUTF8(acc->EMailAddress.c_str()));

	chcIncProto->Select(find_prot_item_index(acc->Incoming.Protocol, true));
	chkIncSsl->SetValue(acc->Incoming.IsSsl);
	txtIncServer->SetValue(wxString::FromUTF8(acc->Incoming.Server));
	txtIncPort->SetValue(acc->Incoming.Port > 0 ? std::to_string(acc->Incoming.Port) : "");
	txtIncUser->SetValue(wxString::FromUTF8(acc->Incoming.UserName));
	chcIncAuth->Select(find_auth_item_index(acc->Incoming.AuthType, acc->Incoming.AuthSpec));

	chcOutProto->Select(find_prot_item_index(acc->Outgoing.Protocol, false));
	chkOutSsl->SetValue(acc->Outgoing.IsSsl);
	txtOutServer->SetValue(wxString::FromUTF8(acc->Outgoing.Server));
	txtOutPort->SetValue(acc->Outgoing.Port > 0 ? std::to_string(acc->Outgoing.Port) : "");
	txtOutUser->SetValue(wxString::FromUTF8(acc->Outgoing.UserName));
	chcOutAuth->Select(find_auth_item_index(acc->Outgoing.AuthType, acc->Outgoing.AuthSpec));

	if (is_new) delete acc;
	btnOk->Enable(true);
}

std::vector<wxString> MailAccCfg::ValidateViewData()
{
	std::vector<wxString> errors;
	if (!check_text_value_required(txtAccName->GetValue()) && !check_text_value_required(txtEmailAddr->GetValue())
		&& !check_text_value_required(txtIncUser->GetValue()) && !check_text_value_required(txtOutUser->GetValue()))
		errors.push_back(Msg_AccDataRequiredValues);
	if (!check_port_value(txtIncPort->GetValue()) || !check_port_value(txtOutPort->GetValue()))
		errors.push_back(Msg_AccDataPortValueInvalid);
	return errors;
}

void MailAccCfg::ShowValidationErrors(const AccountSettings* acc, const std::vector<wxString>& errors)
{
	wxString error_list;
	for (const auto& item : errors) {
		error_list += " - " + item + "\n";
	}
	wxMessageBox(
		wxString::Format(Msg_AccDataValidationError,
			(int)acc->Id, wxString::FromUTF8(acc->GetName()), error_list),
		AppDef_Title, wxICON_ERROR | wxOK, this);
}

bool MailAccCfg::CheckAndSaveViewData(int sel_idx)
{
	if (sel_idx < 0) return true;
	auto acc = FindAccount(sel_idx);
	auto errors = ValidateViewData();
	if (!errors.empty()) {
		ShowValidationErrors(acc, errors);
		return false;
	}

	bool is_changed = false;

	is_changed |= set_text_value(acc->AccountName, txtAccName);
	is_changed |= set_text_value(acc->EMailAddress, txtEmailAddr);

	is_changed |= set_prot_value(acc->Incoming.Protocol, chcIncProto, true);
	is_changed |= set_bool_value(acc->Incoming.IsSsl, chkIncSsl);
	is_changed |= set_text_value(acc->Incoming.Server, txtIncServer);
	is_changed |= set_port_value(acc->Incoming.Port, txtIncPort);
	is_changed |= set_text_value(acc->Incoming.UserName, txtIncUser);
	is_changed |= set_auth_value(acc->Incoming.AuthType, acc->Incoming.AuthSpec, chcIncAuth);

	is_changed |= set_prot_value(acc->Outgoing.Protocol, chcOutProto, false);
	is_changed |= set_bool_value(acc->Outgoing.IsSsl, chkOutSsl);
	is_changed |= set_text_value(acc->Outgoing.Server, txtOutServer);
	is_changed |= set_port_value(acc->Outgoing.Port, txtOutPort);
	is_changed |= set_text_value(acc->Outgoing.UserName, txtOutUser);
	is_changed |= set_auth_value(acc->Outgoing.AuthType, acc->Outgoing.AuthSpec, chcOutAuth);

	if (is_changed && accMgr.SetAccountModified(acc->Id)) {
		chcAccount->SetString(sel_idx, chcAccount->GetString(sel_idx) + AccChangeIndicator);
	}
	return true;
}

bool MailAccCfg::GetChangeInfo(wxString& info)
{
	info.Clear();
	auto changes = accMgr.GetEditState();
	bool is_changed = changes.size() > 0;
	int state_value = changes[AccEditMgr::EditState::esCreated];
	if (state_value > 0) info += wxString::Format(Inf_AccCreated, state_value);
	state_value = changes[AccEditMgr::EditState::esModified];
	if (state_value > 0) {
		if (!info.IsEmpty()) info += Inf_Delimiter;
		info += wxString::Format(Inf_AccModified, state_value);
	}
	state_value = changes[AccEditMgr::EditState::esDeleted];
	if (state_value > 0) {
		if (!info.IsEmpty()) info += Inf_Delimiter;
		info += wxString::Format(Inf_AccDeleted, state_value);
	}
	return is_changed;
}

void MailAccCfg::ApplyChanges()
{
	wxBeginBusyCursor();
	if (0 > accMgr.ApplyChanges())
		wxMessageBox(Msg_GeneralSaveError, AppDef_Title, wxICON_ERROR | wxOK, this);
	wxEndBusyCursor();
}

void MailAccCfg::MailAccCfgUI_OnInitDialog(wxInitDialogEvent& event)
{
	wxBeginBusyCursor();
	LoadAccounts();
	wxEndBusyCursor();
}

void MailAccCfg::toolAccCreate_OnToolClicked(wxCommandEvent& event)
{
	if (!CheckAndSaveViewData(curSelAccIdx)) return;
	int acc_id = accMgr.CreateAccount();
	std::string acc_inf = NewAccNamePrefix;
	acc_inf += std::to_string(acc_id);
	acc_inf += AccChangeIndicator;
	int sel_idx = chcAccount->Append(acc_inf, (void*)acc_id);
	chcAccount->SetSelection(sel_idx);
	LoadViewData(sel_idx);
}

void MailAccCfg::toolAccDelete_OnToolClicked(wxCommandEvent& event)
{
	int sel_idx = chcAccount->GetSelection();
	AccountSettings* acc = FindAccount(sel_idx);
	if (acc && wxOK == wxMessageBox(
		wxString::Format(Msg_AccDeleteQuestion, (int)acc->Id, wxString::FromUTF8(acc->GetName())),
		AppDef_Title, wxICON_QUESTION | wxOK | wxCANCEL | wxCANCEL_DEFAULT, this))
	{
		accMgr.DeleteAccount(acc->Id);
		chcAccount->Delete(sel_idx);
		chcAccount->SetSelection(-1);
		LoadViewData(-1);
	}
}

void MailAccCfg::chcAccount_OnChoice(wxCommandEvent& event)
{
	wxBeginBusyCursor();
	if (CheckAndSaveViewData(curSelAccIdx))
		LoadViewData(chcAccount->GetSelection());
	else
		chcAccount->SetSelection(curSelAccIdx);
	wxEndBusyCursor();
}

void MailAccCfg::btnOk_OnButtonClick(wxCommandEvent& event)
{
	//EndModal(wxID_OK);
	if (!CheckAndSaveViewData(chcAccount->GetSelection())) {
		return;
	}
	ApplyChanges();
	event.Skip();
}

void MailAccCfg::btnNo_OnButtonClick(wxCommandEvent& event)
{
	//EndModal(wxID_CANCEL);
	// Not saving any changes
	event.Skip();
}

void MailAccCfg::MailAccCfgUI_OnClose(wxCloseEvent& event)
{
	if (!CheckAndSaveViewData(chcAccount->GetSelection())) {
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
			case wxNO: // Just not saving any changes
				break;
			case wxCANCEL: event.Veto(); return;
		}
	event.Skip();
}
