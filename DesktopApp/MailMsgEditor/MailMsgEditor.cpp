#pragma once
#include "MailMsgEditor.h"
#include <LisCommon/StrUtils.h>
#include "../../CoreMailLib/MimeHeaderDef.h"
#include "../../CoreMailLib/MimeNodeProc.h"
#include "../../CoreMailLib/MimeParser.h"
#include "../../CoreAppLib/MailMsgFileDef.h"

#define AccountId_Empty MailMsgGrpId_Empty

namespace MailMsgEditor_Def
{
	const wxChar* WndTitle = _T("Mail edit");
}
using namespace MailMsgEditor_Def;

namespace MailMsgEditor_Imp
{
	static void load_accounts(const AccountCfg& src, wxItemContainer* dst, int sel_acc_id);
}
using namespace MailMsgEditor_Imp;

MailMsgEditor::MailMsgEditor(wxWindow* parent) : MailMsgEditorUI(parent)
{
	accCfgSubId = AccCfg.EventSubscribe(AccountCfg_Def::EventType::etAccountsChanged,
		std::bind(&MailMsgEditor::AccountCfg_EventHandler,
			this, std::placeholders::_1, std::placeholders::_2));
	load_accounts(AccCfg, chcSender, AccountId_Empty);
	UpdateToolState(true);
}

MailMsgEditor::~MailMsgEditor()
{
	AccCfg.EventUnsubscribe(accCfgSubId);
}

int MailMsgEditor::OnMailMsgFileSet()
{
	int result = 0;
	if (mailMsgFile && mailMsgFile->GetFilePath()) {
		MimeNode msg_node;
		result = mailMsgFile->LoadData(msg_node, false);
		//MimeNodeProc::NodeInfoContainer nodeStruct;
		if (result >= 0) {
			//result = MimeNodeProc::GetNodeStructInfo(msg_node, nodeStruct, nullptr);
		}
		if (result >= 0) {
			LoadMsgHdrData(&msg_node);
			LoadMsgBodyData(&msg_node);
		}
	} else {
		int acc_id = mailMsgFile ? mailMsgFile->GetGrpId() : AccountId_Empty;
		chcSender->SetSelection(FindSenderIdx(acc_id));
		wxCommandEvent evt;
		chcSender_OnChoice(evt);
	}
	if (result < 0) {
		// TODO: handle the error after the file load attempt - show/log the error
	}
	UpdateEditState();
	UpdateToolState(GetCanEdit());
	return result;
}

bool MailMsgEditor::GetCanEdit()
{
	return txtContent->IsEditable();
}

void MailMsgEditor::SetCanEdit(bool new_state)
{
	UpdateToolState(new_state);
	txtRecipient->SetEditable(new_state);
	txtSubject->SetEditable(new_state);
	txtContent->SetEditable(new_state);
}

int MailMsgEditor::AccountCfg_EventHandler(const AccountCfg* acc_cfg, const AccountCfg::EventInfo& evt_info)
{
	if (AccountCfg_Def::EventType::etAccountsChanged != evt_info.type) return 0;
	int cur_acc_id = GetAccountId();
	for (const auto acc_id : evt_info.data->DeletedAccIds) {
		if (acc_id == cur_acc_id) {
			if (mailMsgFile && (cur_acc_id == mailMsgFile->GetGrpId())) {
				mailMsgFile = nullptr;
			}
			cur_acc_id = AccountId_Empty;
			break;
		}
	}
	load_accounts(AccCfg, chcSender, cur_acc_id);
	UpdateToolState(GetCanEdit());
	return 0;
}

const int MailMsgEditor::GetAccountId(int sel_idx) const
{
	if (sel_idx < 0) sel_idx = chcSender->GetSelection();
	return sel_idx >= 0 ? (int)chcSender->GetClientData(sel_idx) : AccountId_Empty;
}

const AccountSettings* MailMsgEditor::FindAccount(int sel_idx) const
{
	if (sel_idx < 0) return nullptr;
	return AccCfg.FindAccount(GetAccountId(sel_idx));
}

const int MailMsgEditor::FindSenderIdx(int acc_id, const char* mailbox) const
{
	for (unsigned int i = 0; i < chcSender->GetCount(); ++i) {
		bool is_match = (acc_id >= 0) && (acc_id == (int)chcSender->GetClientData(i));
		if (!is_match && mailbox) {
			auto acc = FindAccount(i);
			is_match = (0 == LisStr::StrICmp(acc->GetMailbox(), mailbox));
		}
		if (is_match) return i;
	}
	return -1;
}

void MailMsgEditor::LoadMsgHdrData(const MimeNode* msg_node)
{
	int sender_idx = -1;
	if (mailMsgFile) sender_idx = FindSenderIdx(mailMsgFile->GetGrpId());
	else {
		auto sender_fld = msg_node->Header.GetField(MailHdrName_From);
		if (sender_fld.GetRaw()) {
			auto sender_addr = RfcHeaderFieldCodec::ReadMsgId(sender_fld.GetRaw());
			sender_idx = FindSenderIdx(AccountId_Empty, sender_addr.c_str());
		}
	}
	chcSender->Select(sender_idx);

	txtRecipient->SetValue(msg_node->Header.GetField(MailHdrName_To).GetText());
	txtSubject->SetValue(msg_node->Header.GetField(MailHdrName_Subj).GetText());
}

void MailMsgEditor::LoadMsgBodyData(const MimeNode* msg_node)
{
	txtContent->SetValue(msg_node->Body);
}

void MailMsgEditor::SaveMsgHdrData(MimeNode& msg_node)
{
	msg_node.Header.SetField(MailHdrName_From, new std::basic_string<TCHAR>(chcSender->GetStringSelection()));
	// field ? Sender
	// TODO: the address list should be composed according to the specification...
	// ? RfcHeaderFieldCodec.WriteAddresses // RFC 5322 - 3.4. Address Specification, Appendix A.1. Addressing Examples
	msg_node.Header.SetField(MailHdrName_To, new std::basic_string<TCHAR>(txtRecipient->GetValue()));
	// fields ? Cc, Bcc
	msg_node.Header.SetField(MailHdrName_Subj, new std::basic_string<TCHAR>(txtSubject->GetValue()));
	msg_node.Header.SetField(MailHdrName_Date, MimeHeaderTimeValueUndefined);
	// ...
	msg_node.Header.SetField(MailHdrName_MimeVersion, new std::string(MailHdrData_MimeVersion1));
}

void MailMsgEditor::SaveMsgBodyData(MimeNode& msg_node)
{
	msg_node.Header.SetField(MailHdrName_ContentType, new std::string("text/plain; charset=utf-8"));
	msg_node.Header.SetField(MailHdrName_ContentTransferEncoding, new std::string("8bit"));

	msg_node.Body = txtContent->GetValue().ToUTF8();
}

void MailMsgEditor::UpdateEditState()
{
	auto sender_idx = chcSender->GetCurrentSelection();
	chcSender->Enable((mailMsgFile == nullptr) || (mailMsgFile->GetFilePath() == nullptr)
		|| (sender_idx <= 0));
}

void MailMsgEditor::UpdateToolState(bool can_edit)
{
	auto acc = FindAccount(chcSender->GetSelection());
	tlbrMain->EnableTool(toolSaveFile->GetId(), can_edit && mailMsgFile && acc);
	tlbrMain->EnableTool(toolSendMail->GetId(), can_edit && acc && !acc->Outgoing.Server.empty() && !txtRecipient->IsEmpty());
}

void MailMsgEditor::toolSaveMessage_OnToolClicked(wxCommandEvent& event)
{
	MimeNode mail_msg;
	SaveMsgHdrData(mail_msg);
	SaveMsgBodyData(mail_msg);
	int result = mailMsgFile->SaveData(mail_msg, GetAccountId());
	// TODO: handle saving errors
	UpdateEditState();
	UpdateToolState(GetCanEdit());
}

void MailMsgEditor::toolSendMessage_OnToolClicked(wxCommandEvent& event)
{
	// TODO: save the message data if needed
	mailMsgFile->SetMailToSend();
	// TODO: close the editor window
}

void MailMsgEditor::chcSender_OnChoice(wxCommandEvent& event)
{
	UpdateToolState(GetCanEdit());
}

void MailMsgEditor::mnuAttachmentFileSave_OnMenuSelection(wxCommandEvent& event)
{
}

static void MailMsgEditor_Imp::load_accounts(const AccountCfg& src, wxItemContainer* dst, int sel_acc_id)
{
	dst->Clear();
	AccountCfg::AccountsIterator acc_list_begin, acc_list_end;
	src.GetIter(acc_list_begin, acc_list_end);
	int sel_idx = -1;
	for (auto it = acc_list_begin; it != acc_list_end; ++it) {
		wxString name(wxString::FromUTF8(it->GetName()));
		if (name.IsEmpty()) name = "#" + wxString::Format("%i", it->Id);
		name += " <";
		name += it->GetMailbox();
		name += ">";
		int idx = dst->Append(name, (void*)it->Id);
		if (sel_acc_id == it->Id) sel_idx = idx;
	}
	if (sel_idx >= 0) dst->Select(sel_idx);
}
