#pragma once
#include "MailMainViewUI.h"
#include <mutex>
#include <LisCommon/Logger.h>
#include "../../CoreAppLib/AccountConfig.h"
#include "../MailMsgFileMgr.h"
#include "../MailMsgViewMgr.h"
#include "MailEventRouter.h"

namespace MailMainView_Def
{
	extern const wxChar* WndTitle;
}

class MailMainView : public MailMainViewUI
{
	typedef std::function<void(wxWindow* wnd, const TCHAR* title)> ViewCreationHandler;
private:
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	bool masterModelViewOption1 = false;
	MailMsgFileMgr* msgFileMgr;
	MailMsgViewMgr* msgViewMgr;

	void AdjustMailSyncUiControls(MailSyncProcStatus acc_busy_state);
	int AccountCfg_EventHandler(const AccountConfig* acc_cfg, const AccountConfig::EventInfo& evt_info);
	void RefreshMasterToolsState(const wxDataViewItem* item = nullptr);
	void RefreshDetailToolsState(bool enable_filter);
	void StartMailSync(bool receive, bool send);
	void StopMailSync(bool receiving, bool sending);

	void mnuMailSyncStartRecv_OnMenuSelection(wxCommandEvent& event);
	void mnuMailSyncStartSend_OnMenuSelection(wxCommandEvent& event);
	void mnuMailSyncStopRecv_OnMenuSelection(wxCommandEvent& event);
	void mnuMailSyncStopSend_OnMenuSelection(wxCommandEvent& event);

	// ****** MainViewUI override ******
	virtual void toolMasterViewConfig_OnToolClicked(wxCommandEvent& event) override;
	virtual void toolStartSyncMail_OnToolClicked(wxCommandEvent& event); // not override however related
	virtual void toolStopSyncMail_OnToolClicked(wxCommandEvent& event); // not override however related
	virtual void toolMailMsgCreate_OnToolClicked(wxCommandEvent& event) override;
	virtual void dvAccFolders_OnDataViewCtrlSelectionChanged(wxDataViewEvent& event) override;
	virtual void toolMailMsgFilterSwitch_OnToolClicked(wxCommandEvent& event) override;
	virtual void cmbMailMsgFilterValue_OnKeyDown(wxKeyEvent& event) override;
	virtual void cmbMailMsgFilterValue_OnText(wxCommandEvent& event) override;
	virtual void cmbMailMsgFilterValue_OnTextEnter(wxCommandEvent& event) override;
	virtual void toolMailMsgFilterApply_OnToolClicked(wxCommandEvent& event) override;
	virtual void toolMailMsgLayout_OnToolClicked(wxCommandEvent& event) override;
	virtual void dvMailMsgList_OnDataViewCtrlItemActivated(wxDataViewEvent& event) override;
	virtual void dvMailMsgList_OnDataViewCtrlSelectionChanged(wxDataViewEvent& event) override;
	virtual void dvMailMsgList_OnDataViewCtrlItemContextMenu(wxDataViewEvent& event) override;
	virtual void mnuMailMsgItemOpen_OnMenuSelection(wxCommandEvent& event) override;
	virtual void mnuMailMsgItemMarkAsRead_OnMenuSelection(wxCommandEvent& event) override;
	virtual void mnuMailMsgItemMarkUnread_OnMenuSelection(wxCommandEvent& event) override;
	virtual void mnuMailMsgItemDelete_OnMenuSelection(wxCommandEvent& event) override;

	// ****** Master ******
	int GetCurrentAccountId();
	void CreateMasterViewModel(bool group_by_folder);
	void ExpandFirstLevel();
	static bool IsFolderMatches(int folder_id, MailMsgFile* mail_msg);
	MailSyncProcStatus GetAccItemBusyState(const wxDataViewItem& item);
	static void ResetFolderMailCount(wxDataViewCtrl* view_ctrl, int folder_id);

	// ****** Detail ******
	bool isMsgFilterCaseSensitive = false;
	wxString mailMsgFilterValue;
	void CreateDetailViewModel(const wxDataViewItem* master_item);
	bool ApplyMailMsgFilter(const wxString& value);
	static bool IsFilterMatches(const wxString& filter_value, MailMsgFile* mail_msg, bool case_sensitive);
	void SetMailMessageReadStatus(wxDataViewItemArray& items, bool is_read);
	void DeleteMailMessages(wxDataViewItemArray& items);
	void OpenMailMsgItem(const wxDataViewItem* mail_msg_item);

	// ****** MailMsgProcEvent ******
	MailEventRouter mailEvtRtr;
	void InitMailMsgProcEvent();
	void FreeMailMsgProcEvent();
	void MailMsgCommandHandler(wxCommandEvent& event);
	void MailMsgEvent_CredentialsRequest(
		const Connections::ConnectionInfo* connection, std::string* pswd_data, bool* need_save);
	void MailMsgEvent_NewMessageAdded(std::shared_ptr<MailMsgFile>& mail_msg);
	void MailMsgEvent_SyncFinished();
public:
	MailMainView(wxWindow* parent, MailMsgFileMgr* msg_file_mgr, MailMsgViewMgr* msg_view_mgr);
	~MailMainView();
};
