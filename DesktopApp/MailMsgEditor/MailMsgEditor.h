#pragma once
#include "MailMsgEditorUI.h"
#include "../MailMsgFileView.h"
#include "../../CoreMailLib/MimeNode.h"
#include "../../CoreAppLib/AccountCfg.h"

namespace MailMsgEditor_Def
{
	extern const TCHAR* WndTitle;
}

class MailMsgEditor : public MailMsgEditorUI, public MailMsgFileView
{
	AccountCfg::EventSubscriptionId accCfgSubId;
	int AccountCfg_EventHandler(const AccountCfg* acc_cfg, const AccountCfg::EventInfo& evt_info);
	const int GetAccountId(int sel_idx = -1) const;
	const AccountSettings* FindAccount(int sel_idx) const;
	const int FindSenderIdx(int acc_id = -1, const char* mailbox = nullptr) const;

	void LoadMsgHdrData(const MimeNode* msg_node);
	void LoadMsgBodyData(const MimeNode* msg_node);
	void SaveMsgHdrData(MimeNode& msg_node);
	void SaveMsgBodyData(MimeNode& msg_node);

	void UpdateEditState();
	void UpdateToolState(bool can_edit);

	// ****** MailMsgEditorUI override ******
	virtual void toolSaveMessage_OnToolClicked(wxCommandEvent& event) override;
	virtual void toolSendMessage_OnToolClicked(wxCommandEvent& event) override;
	virtual void chcSender_OnChoice(wxCommandEvent& event) override;
	virtual void mnuAttachmentFileSave_OnMenuSelection(wxCommandEvent& event) override;

	// ****** MaiMsgFileView override ******
	virtual int OnMailMsgFileSet() override;
public:
	MailMsgEditor(wxWindow* parent);
	virtual ~MailMsgEditor();

	// ****** MaiMsgFileView override ******
	virtual bool GetCanEdit() override;
	virtual void SetCanEdit(bool new_state) override;
};
