#pragma once
#include "MailMsgEditor.h"
#include <LisCommon/StrUtils.h>
#include "../../CoreAppLib/MailMsgFileDef.h"

#define AccountId_Empty MailMsgGrpId_Empty

namespace MailMsgEditor_Imp
{
	static void load_accounts(const AccountCfg& src, wxItemContainer* dst, int sel_acc_id);
}
using namespace MailMsgEditor_Imp;

void MailMsgEditor::AccountInfo_Setup()
{
	accCfgSubId = AccCfg.EventSubscribe(AccountCfg_Def::EventType::etAccountsChanged,
		std::bind(&MailMsgEditor::AccCfg_EventHandler,
			this, std::placeholders::_1, std::placeholders::_2));
	load_accounts(AccCfg, chcSender, AccountId_Empty);
}

void MailMsgEditor::AccountInfo_Cleanup()
{
	AccCfg.EventUnsubscribe(accCfgSubId);
}

int MailMsgEditor::AccCfg_EventHandler(const AccountCfg* acc_cfg, const AccountCfg::EventInfo& evt_info)
{
	if (AccountCfg_Def::EventType::etAccountsChanged != evt_info.type) return 0;
	const auto& del_acc_ids = evt_info.data->DeletedAccIds;
	int cur_acc_id = GetAccountId();
	if (std::find(del_acc_ids.begin(), del_acc_ids.end(), cur_acc_id) != del_acc_ids.end()) {
		// Current message account has been deleted - clear all the references
		if (mailMsgFile && (cur_acc_id == mailMsgFile->GetGrpId())) {
			mailMsgFile = nullptr;
		}
		cur_acc_id = AccountId_Empty;
	}
	load_accounts(AccCfg, chcSender, cur_acc_id);
	if (!mailMsgFile) {
		this->OnMailMsgFileSet();
	}
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

void MailMsgEditor::chcSender_OnChoice(wxCommandEvent& event)
{
	UpdateToolState(GetCanEdit());
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
