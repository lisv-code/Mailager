#pragma once
#include "MailMsgEditor.h"
#include <LisCommon/StrUtils.h>
#include "../../CoreMailLib/MimeParser.h"
#include "../../CoreAppLib/AccountCfg.h"
#include "../../CoreAppLib/AuthTokenProc.h"
#include "../../CoreAppLib/MailMsgFileHelper.h"
#include "../../CoreAppLib/MailMsgTransmitter.h" // ?
#include "../AppCfg.h"

#define AccountId_Empty -1

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
	UpdateToolbar();
}

MailMsgEditor::~MailMsgEditor()
{
	AccCfg.EventUnsubscribe(accCfgSubId);
}

int MailMsgEditor::OnMailMsgFileSet()
{
	int result = 0;
	if (mailMsgFile && mailMsgFile->GetFilePath()) {
		result = LoadData(mailMsgFile->GetFilePath());
		if (result >= 0) UpdateHeaderView();
	} else {
		int acc_id = mailMsgFile ? mailMsgFile->GetGrpId() : AccountId_Empty;
		chcSender->SetSelection(FindSenderIdx(acc_id));
		wxCommandEvent evt;
		chcSender_OnChoice(evt);
	}
	return result;
}

int MailMsgEditor::AccountCfg_EventHandler(const AccountCfg* acc_cfg, const AccountCfg::EventInfo& evt_info)
{
	if (AccountCfg_Def::EventType::etAccountsChanged != evt_info.type) return 0;
	int cur_acc_id = GetAccountId();
	for (const auto acc_id : evt_info.data->DeletedAccIds) {
		if (acc_id == cur_acc_id) {
			cur_acc_id = AccountId_Empty;
			if (mailMsgFile) mailMsgFile = nullptr;
			break;
		}
	}
	load_accounts(AccCfg, chcSender, cur_acc_id);
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
		bool is_this = (acc_id >= 0) && (acc_id == (int)chcSender->GetClientData(i));
		if (!is_this && mailbox) {
			auto acc = FindAccount(i);
			is_this = (0 == LisStr::StrICmp(acc->GetMailbox(), mailbox));
		}
		if (is_this) return i;
	}
	return -1;
}

int MailMsgEditor::LoadData(const FILE_PATH_CHAR* msg_file_path)
{
	nodeStruct.clear();

	std::ifstream stm;
	int result = MailMsgFileHelper::InitInputStream(stm, msg_file_path, true);
	if (result < 0) return result;

	MimeParser parser;
	result = parser.Load(stm, false);
	stm.close();
	if (result >= 0) {
		result = parser.GetData(msgNode, hvtAuto);
		if (result >= 0) {
			result = MimeNodeProc::GetNodeStructInfo(msgNode, nodeStruct, nullptr);
		}
	}
	return result;
}

void MailMsgEditor::UpdateHeaderView()
{
	int sender_idx = -1;
	if (mailMsgFile) sender_idx = FindSenderIdx(mailMsgFile->GetGrpId());
	else {
		auto sender_fld = msgNode.Header.GetField(MailMsgHdrName_From);
		if (sender_fld.GetRaw()) {
			auto sender_addr = RfcHeaderFieldCodec::GetMessageId(sender_fld.GetRaw()).id;
			sender_idx = FindSenderIdx(AccountId_Empty, sender_addr.c_str());
		}
	}
	chcSender->Select(sender_idx);
	chcSender->Enable(mailMsgFile == nullptr || sender_idx <= 0);

	txtRecipient->SetValue(msgNode.Header.GetField(MailMsgHdrName_To).GetText());
	txtSubject->SetValue(msgNode.Header.GetField(MailMsgHdrName_Subj).GetText());
}

void MailMsgEditor::UpdateToolbar()
{
	auto acc = FindAccount(chcSender->GetSelection());
	tlbrMain->EnableTool(toolSaveMessage->GetId(), nullptr != acc);
	tlbrMain->EnableTool(toolSendMessage->GetId(), acc && !acc->Outgoing.Server.empty());
}

void MailMsgEditor::toolSaveMessage_OnToolClicked(wxCommandEvent& event)
{
	// msgNode to be updated
	// msgFile should be created somewhere on this stage if not exists yet
	UpdateHeaderView();
}

void MailMsgEditor::toolSendMessage_OnToolClicked(wxCommandEvent& event)
{
	// TODO: ! following test implementation needs replacement

	auto acc = FindAccount(chcSender->GetSelection());
	MailMsgTransmitter transmitter;
	transmitter.SetLocation(AppCfg.Get().AppDataDir.c_str(), acc->Directory.c_str(), acc->Outgoing, acc->Id);

	char buf[0xFFF] = { 0 };
	AuthTokenProc::ComposeAuthPlainToken(buf, nullptr, acc->Outgoing.UserName.c_str(), "<PASSWORD>");

	transmitter.Transmit(buf, nullptr);
}

void MailMsgEditor::chcSender_OnChoice(wxCommandEvent& event)
{
	UpdateToolbar();
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
