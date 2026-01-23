#pragma once
#include "MailMsgEditorUI.h"
#include "../MailMsgFileView.h"
#include "../../CoreMailLib/MimeNode.h"
#include "../../CoreAppLib/AccountCfg.h"
#include "../MailMsgCtrlAttachments/MailMsgCtrlAttachments.h"

namespace MailMsgEditor_Def
{
	extern const TCHAR* WndTitle;
}

class MailMsgEditor : public MailMsgEditorUI, public MailMsgFileView
{
	/****** Account (Sender) ******/
	AccountCfg::EventSubscriptionId accCfgSubId;
	void AccountInfo_Setup();
	void AccountInfo_Cleanup();
	int AccCfg_EventHandler(const AccountCfg* acc_cfg, const AccountCfg::EventInfo& evt_info);
	const int GetAccountId(int sel_idx = -1) const;
	const AccountSettings* FindAccount(int sel_idx) const;
	const int FindSenderIdx(int acc_id = -1, const char* mailbox = nullptr) const;
	void RefreshSender();

	/****** Attachments ******/
	MailMsgCtrlAttachments attachmentsCtrl;

	/****** Message metadata and content ******/
	void LoadMsgHdrData(const MimeNode* msg_node);
	int LoadMsgBodyData(const MimeNode* msg_node);
	void SaveMsgHdrData(MimeNode& msg_node);
	void SaveMsgBodyData(MimeNode& msg_node, bool pass_attachments_ownership);

	void RefreshSenderState();
	void RefreshToolState();

	// ****** MailMsgEditorUI override ******
	virtual void toolSaveMessage_OnToolClicked(wxCommandEvent& event) override;
	virtual void toolSendMessage_OnToolClicked(wxCommandEvent& event) override;
	virtual void toolAddAttachment_OnToolClicked(wxCommandEvent& event) override;
	virtual void chcSender_OnChoice(wxCommandEvent& event) override;
	virtual void mnuAttachmentsAdd_OnMenuSelection(wxCommandEvent& event) override;

	// ****** MaiMsgFileView override ******
	virtual int OnMailMsgFileChanged(MailMsgFile* prev_value) override;
public:
	MailMsgEditor(wxWindow* parent);
	virtual ~MailMsgEditor();

	// ****** MaiMsgFileView override ******
	virtual bool GetCanEdit() override;
	virtual void SetCanEdit(bool new_state) override;
};
