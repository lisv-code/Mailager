#pragma once
#include "MailMsgEditorUI.h"
#include "../MailMsgFileView.h"
#include "../../CoreAppLib/AccountCfg.h"
#include "../../CoreAppLib/MailMsgFile.h"
#include "../../CoreMailLib/MimeNodeProc.h"

namespace MailMsgEditor_Def
{
	extern const TCHAR* WndTitle;
}

class MailMsgEditor : public MailMsgEditorUI, public MailMsgFileView
{
	MimeNode msgNode;
	MimeNodeProc::NodeInfoContainer nodeStruct;

	AccountCfg::EventSubscriptionId accCfgSubId;
	int AccountCfg_EventHandler(const AccountCfg* acc_cfg, const AccountCfg::EventInfo& evt_info);
	const int GetAccountId(int sel_idx = -1) const;
	const AccountSettings* FindAccount(int sel_idx) const;
	const int FindSenderIdx(int acc_id = -1, const char* mailbox = nullptr) const;

	int LoadData(const FILE_PATH_CHAR* msg_file_path);
	void UpdateHeaderView();

	void UpdateToolbar();

	// ****** MailMsgEditorUI override ******
	virtual void toolSaveMessage_OnToolClicked(wxCommandEvent& event) override;
	virtual void toolSendMessage_OnToolClicked(wxCommandEvent& event) override;
	virtual void chcSender_OnChoice(wxCommandEvent& event) override;
	virtual void mnuAttachmentFileSave_OnMenuSelection(wxCommandEvent& event) override;

	// ****** MaiMsgFileView override ******
	virtual int OnMailMsgFileSet();
public:
	MailMsgEditor(wxWindow* parent);
	virtual ~MailMsgEditor();

	// ****** MaiMsgFileView override ******
	virtual bool GetCanEdit() { return true; };
	virtual int SetCanEdit(bool new_state) { return -1; }
};
