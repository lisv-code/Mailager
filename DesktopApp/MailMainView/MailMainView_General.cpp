#include "MailMainView.h"
#include <vector>
#include <wx/tglbtn.h>
#include "../../CoreAppLib/AccountCfg.h"
#include "../../CoreAppLib/AppDef.h"
#include "../ResMgr.h"
#include "MasterViewModel.h"
#include "DetailViewModel.h"

namespace MailMainView_Def
{
#define Log_Scope "MailMain_Gen"

	const TCHAR* WndTitle = _T("Mail main view");
	const TCHAR* Msg_MailDeleteQuestion = _T("Delete selected message(s)?\n\n%s");
	const TCHAR* Msg_MailStopSyncQuestion = _T("Stop mail synchronization for %i account(s)?");
}
using namespace MailMainView_Def;

MailMainView::MailMainView(wxWindow* parent, MailMsgFileMgr* msg_file_mgr, MailMsgViewMgr* msg_view_mgr)
	: MailMainViewUI(parent)
{
	msgFileMgr = msg_file_mgr;
	msgViewMgr = msg_view_mgr;
	msgViewMgr->SetEmbViewDefaults(pnlMailMsgView);

	InitMailMsgProcEvent();

	AccCfg.EventSubscribe(AccountCfg_Def::etAccountsChanged,
		std::bind(&MailMainView::AccountCfg_EventHandler,
			this, std::placeholders::_1, std::placeholders::_2));
	CreateMasterViewModel(masterModelViewOption1);
	tlbrMaster->RemoveTool(toolStopSyncMail->GetId()); // needs initial state update
	RefreshMasterToolsState();
	RefreshDetailToolsState(false);
#ifdef _WINDOWS
	auto row_height = dvMailMsgList->GetColumn(0)->GetRenderer()->GetSize().y;
#else
	auto row_height = 26; // GTK: looks like it's set automatically by the control
#endif
	dvMailMsgList->SetRowHeight(2 + row_height * 2);
}

MailMainView::~MailMainView()
{
	msgViewMgr->InitEmbView(nullptr);
	msgViewMgr->SetEmbViewDefaults(nullptr);
	FreeMailMsgProcEvent();
}

void MailMainView::toolConfigMasterView_OnToolClicked(wxCommandEvent& event)
{
	wxBeginBusyCursor();
	CreateMasterViewModel(!masterModelViewOption1);
	masterModelViewOption1 = !masterModelViewOption1;
	wxEndBusyCursor();
}

void MailMainView::toolStartSyncMail_OnToolClicked(wxCommandEvent& event)
{
	wxBeginBusyCursor();
	const auto item = dvAccFolders->GetSelection();
	if (item.IsOk()) {
		auto data_item = (MasterViewModel::DataItem*)item.m_pItem;
		auto accounts = data_item->GetAccounts();
		for (auto& account : accounts) {
			bool result = msgFileMgr->StartMailSync(account->Id, *account);
		}
		RefreshMasterToolsState(&item);
	}
	wxEndBusyCursor();
}

void MailMainView::toolStopSyncMail_OnToolClicked(wxCommandEvent& event)
{
	wxBeginBusyCursor();
	const auto item = dvAccFolders->GetSelection();
	if (item.IsOk()) {
		auto data_item = (MasterViewModel::DataItem*)item.m_pItem;
		auto accounts = data_item->GetAccounts();
		if (accounts.size() > 0
			&& wxOK == wxMessageBox(
				wxString::Format(Msg_MailStopSyncQuestion, (int)accounts.size()), AppDef_Title,
				wxICON_QUESTION | wxOK | wxCANCEL | wxCANCEL_DEFAULT, this))
		{
			for (auto account : accounts) {
				logger->LogFmt(LisLog::llInfo,
					Log_Scope " Stopping acc#%i mail sync...", account->Id);
				bool result = msgFileMgr->StopProcessing(account->Id);
			}
			RefreshMasterToolsState(&item);
		}
	}
	wxEndBusyCursor();
}

void MailMainView::toolCreateMailMsg_OnToolClicked(wxCommandEvent& event)
{
	wxBeginBusyCursor();
	msgViewMgr->OpenStdView(msgFileMgr->CreateMailMsg(GetCurrentAccountId()));
	wxEndBusyCursor();
}

void MailMainView::dvAccFolders_OnDataViewCtrlSelectionChanged(wxDataViewEvent& event)
{
	wxBeginBusyCursor();

	dvMailMsgList->UnselectAll();
	msgViewMgr->InitEmbView(nullptr);
	auto item = event.GetItem();
	if (item && item.m_pItem) {
		RefreshMasterToolsState();
		CreateDetailViewModel(&item);
	}

	wxEndBusyCursor();
}

void MailMainView::toolMailMsgFilterSwitch_OnToolClicked(wxCommandEvent& event)
{
	RefreshDetailToolsState(event.IsChecked());
}

void MailMainView::cmbMailMsgFilterValue_OnKeyDown(wxKeyEvent& event)
{
	if (WXK_ESCAPE == event.GetKeyCode()) {
		tlbrDetail->ToggleTool(toolMailMsgFilterSwitch->GetId(), false);
		RefreshDetailToolsState(false);
	}
	// Note: WXK_RETURN is handled as separated event (OnTextEnter)
	event.Skip();
}

void MailMainView::cmbMailMsgFilterValue_OnText(wxCommandEvent& event)
{
	bool is_filter_changed = !mailMsgFilterValue.IsSameAs(event.GetString(), isMsgFilterCaseSensitive);
	tlbrDetail->EnableTool(toolMailMsgFilterApply->GetId(), is_filter_changed);
}

void MailMainView::cmbMailMsgFilterValue_OnTextEnter(wxCommandEvent& event)
{
	wxCommandEvent cmd;
	toolMailMsgFilterApply_OnToolClicked(cmd);
}

void MailMainView::toolMailMsgFilterApply_OnToolClicked(wxCommandEvent& event)
{
	if (ApplyMailMsgFilter(cmbMailMsgFilterValue->GetValue()))
	{
		if (wxNOT_FOUND == cmbMailMsgFilterValue->FindString(mailMsgFilterValue, isMsgFilterCaseSensitive))
			cmbMailMsgFilterValue->Append(mailMsgFilterValue);
		tlbrDetail->EnableTool(toolMailMsgFilterApply->GetId(), false);
	}
}

void MailMainView::toolMailMsgLayout_OnToolClicked(wxCommandEvent& event)
{
	auto mode = wndDetailSplitter->GetSplitMode();
	mode = wxSPLIT_VERTICAL == mode ? wxSPLIT_HORIZONTAL : wxSPLIT_VERTICAL;
	wndDetailSplitter->SetSplitMode(mode);
	wndDetailSplitter->GetParent()->Layout();
}

void MailMainView::dvMailMsgList_OnDataViewCtrlItemActivated(wxDataViewEvent& event)
{
	wxBeginBusyCursor();
	OpenMailMsgItem(&event.GetItem());
	wxEndBusyCursor();
}

void MailMainView::dvMailMsgList_OnDataViewCtrlSelectionChanged(wxDataViewEvent& event)
{
	wxBeginBusyCursor();

	auto mail_msg = (DetailViewModel::DataItem*)event.GetItem().m_pItem;
	msgViewMgr->InitEmbView(mail_msg ? *mail_msg : std::shared_ptr<MailMsgFile>(nullptr));

	wxEndBusyCursor();
}

void MailMainView::dvMailMsgList_OnDataViewCtrlItemContextMenu(wxDataViewEvent& event)
{
	if (event.GetItem().IsOk()) PopupMenu(mnuMailMsgItem);
}

void MailMainView::mnuMailMsgItemOpen_OnMenuSelection(wxCommandEvent& event)
{
	wxDataViewItemArray items;
	int count = dvMailMsgList->GetSelections(items);
	if (count > 0) {
		wxBeginBusyCursor();
		for (const auto& item : items) OpenMailMsgItem(&item);
		wxEndBusyCursor();
	}
}

void MailMainView::mnuMailMsgItemMarkAsRead_OnMenuSelection(wxCommandEvent& event)
{
	wxDataViewItemArray items;
	int count = dvMailMsgList->GetSelections(items);
	if (count > 0) UpdateMailMessageStatusFlag(items, true, MailMsgStatus::mmsIsSeen);
}

void MailMainView::mnuMailMsgItemMarkUnread_OnMenuSelection(wxCommandEvent& event)
{
	wxDataViewItemArray items;
	int count = dvMailMsgList->GetSelections(items);
	if (count > 0) UpdateMailMessageStatusFlag(items, false, MailMsgStatus::mmsIsSeen);
}

void MailMainView::mnuMailMsgItemDelete_OnMenuSelection(wxCommandEvent& event)
{
	wxDataViewItemArray items;
	int count = dvMailMsgList->GetSelections(items);
	if (count > 0) {
		auto details = count > 1
			? wxString::Format(_T("%i items"), count)
			: wxString::Format(_T("%s\n%s"),
				((DetailViewModel::DataItem*)items[0].m_pItem)->get()->GetInfo().GetField(MailMsgHdrName_From).GetText(),
				((DetailViewModel::DataItem*)items[0].m_pItem)->get()->GetInfo().GetField(MailMsgHdrName_Subj).GetText());
		switch (wxMessageBox(
			wxString::Format(Msg_MailDeleteQuestion, details), AppDef_Title,
			wxICON_QUESTION | wxOK | wxOK_DEFAULT | wxCANCEL, this))
		{
		case wxOK:
			DeleteMailMessages(items);
			break;
		case wxCANCEL:
			break;
		}
	}
}

int MailMainView::AccountCfg_EventHandler(const AccountCfg* acc_cfg, const AccountCfg::EventInfo& evt_info)
{
	wxBeginBusyCursor();
	for (int acc_id : evt_info.data->DeletedAccIds) msgFileMgr->RemoveGroup(acc_id);
	CreateMasterViewModel(masterModelViewOption1);
	CreateDetailViewModel(nullptr);
	wxEndBusyCursor();
	return 0;
}

void MailMainView::RefreshMasterToolsState(const wxDataViewItem* item)
{
	if (!item) {
		item = &dvAccFolders->GetSelection();
	}
	bool is_acc_busy = item ? IsAccItemBusy(*item, msgFileMgr) : true;
	if (is_acc_busy) {
		auto pos = tlbrMaster->GetToolPos(toolStartSyncMail->GetId());
		if (wxNOT_FOUND != pos) {
			tlbrMaster->RemoveTool(toolStartSyncMail->GetId());
			tlbrMaster->InsertTool(pos, toolStopSyncMail);
		}
	} else {
		auto pos = tlbrMaster->GetToolPos(toolStopSyncMail->GetId());
		if (wxNOT_FOUND != pos) {
			tlbrMaster->RemoveTool(toolStopSyncMail->GetId());
			tlbrMaster->InsertTool(pos, toolStartSyncMail);
		}
	}
	tlbrMaster->Realize();
}

void MailMainView::RefreshDetailToolsState(bool enable_filter)
{
	if (enable_filter) {
		auto pos = tlbrDetail->GetToolPos(toolMailMsgFilterSwitch->GetId());
		tlbrDetail->InsertTool(pos + 1, toolMailMsgFilterApply);
		tlbrDetail->EnableTool(toolMailMsgFilterApply->GetId(), false);
		cmbMailMsgFilterValue->Show();
		tlbrDetail->InsertControl(pos + 1, cmbMailMsgFilterValue);
		cmbMailMsgFilterValue->SetFocus();
	} else {
		tlbrDetail->RemoveTool(toolMailMsgFilterApply->GetId());
		tlbrDetail->RemoveTool(cmbMailMsgFilterValue->GetId());
		cmbMailMsgFilterValue->Hide();
		cmbMailMsgFilterValue->SetValue("");
		ApplyMailMsgFilter("");
	}
	tlbrDetail->Realize();
}
