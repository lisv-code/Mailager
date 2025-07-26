#pragma once
#include "MailMainViewUI.h"
#include <mutex>
#include <LisCommon/Logger.h>
#include "../../CoreAppLib/AccountCfg.h"
#include "../MailMsgFileMgr.h"
#include "../MailMsgViewMgr.h"

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

	int AccountCfg_EventHandler(const AccountCfg* acc_cfg, const AccountCfg::EventInfo& evt_info);
	void RefreshMasterToolsState(const wxDataViewItem* item = nullptr);
	void RefreshDetailToolsState(bool enable_filter);

	// ****** MainViewUI override ******
	virtual void toolConfigMasterView_OnToolClicked(wxCommandEvent& event) override;
	virtual void toolStartSyncMail_OnToolClicked(wxCommandEvent& event) override;
	virtual void toolStopSyncMail_OnToolClicked(wxCommandEvent& event) override;
	virtual void toolCreateMailMsg_OnToolClicked(wxCommandEvent& event) override;
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
	static bool IsAccItemBusy(const wxDataViewItem& item, MailMsgFileMgr* msg_mgr);
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
	bool isStop;
	std::mutex mutex1;
	int procResult;
	void InitMailMsgProcEvent();
	void FreeMailMsgProcEvent();
	int MailMsgProcEventHandler(const MailMsgFileMgr* mail_mgr, const MailMsgFileMgr::EventInfo& evt_info);
	static bool NeedProcEventWait(MailMsgFileMgr_EventType evt_type, bool event_finish);
	static bool RouteProcEvent(wxEvtHandler* dest, const MailMsgFileMgr::EventInfo& evt_info);
	void MailMsgCommandHandler(wxCommandEvent& event);
	void MailMsgEvent_CredentialsRequest(
		const Connections::ConnectionInfo* connection, std::string* pswd_data, bool* need_save);
	void MailMsgEvent_NewMessageAdded(std::shared_ptr<MailMsgFile>* mail_msg);
	void MailMsgEvent_SyncFinished();
public:
	MailMainView(wxWindow* parent, MailMsgFileMgr* msg_file_mgr, MailMsgViewMgr* msg_view_mgr);
	~MailMainView();
};
