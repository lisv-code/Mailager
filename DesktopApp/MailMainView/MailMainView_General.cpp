#include "MailMainView.h"
#include <vector>
#include <wx/tglbtn.h>
#include "../../CoreMailLib/MimeHeaderDef.h"
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

	const TCHAR* ToolHlp_MailSyncStart = _T("Start mail sync");
	const TCHAR* MnuLbl_MailSyncStartRecv = _T("Receive mail");
	const TCHAR* MnuLbl_MailSyncStartSend = _T("Send mail");
	const TCHAR* ToolHlp_MailSyncStop = _T("Stop mail sync");
	const TCHAR* MnuLbl_MailSyncStopRecv = _T("Stop receiving");
	const TCHAR* MnuLbl_MailSyncStopSend = _T("Stop sending");
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

	AdjustMailSyncUiControls(MailMsgFileMgr::GrpProcStatus::gpsNone);

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

void MailMainView::toolMasterViewConfig_OnToolClicked(wxCommandEvent& event)
{
	wxBeginBusyCursor();
	CreateMasterViewModel(!masterModelViewOption1);
	masterModelViewOption1 = !masterModelViewOption1;
	wxEndBusyCursor();
}

void MailMainView::toolStartSyncMail_OnToolClicked(wxCommandEvent& event)
{
	StartMailSync(true, true);
}

void MailMainView::toolStopSyncMail_OnToolClicked(wxCommandEvent& event)
{
	StopMailSync(true, true);
}

void MailMainView::mnuMailSyncStartRecv_OnMenuSelection(wxCommandEvent& event)
{
	StartMailSync(true, false);
}

void MailMainView::mnuMailSyncStartSend_OnMenuSelection(wxCommandEvent& event)
{
	StartMailSync(false, true);
}

void MailMainView::mnuMailSyncStopRecv_OnMenuSelection(wxCommandEvent& event)
{
	StopMailSync(true, false);
}

void MailMainView::mnuMailSyncStopSend_OnMenuSelection(wxCommandEvent& event)
{
	StopMailSync(false, true);
}

void MailMainView::toolMailMsgCreate_OnToolClicked(wxCommandEvent& event)
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
	if (count > 0) SetMailMessageReadStatus(items, true);
}

void MailMainView::mnuMailMsgItemMarkUnread_OnMenuSelection(wxCommandEvent& event)
{
	wxDataViewItemArray items;
	int count = dvMailMsgList->GetSelections(items);
	if (count > 0) SetMailMessageReadStatus(items, false);
}

void MailMainView::mnuMailMsgItemDelete_OnMenuSelection(wxCommandEvent& event)
{
	wxDataViewItemArray items;
	int count = dvMailMsgList->GetSelections(items);
	if (count > 0) {
		auto details = count > 1
			? wxString::Format(_T("%i items"), count)
			: wxString::Format(_T("%s\n%s"),
				((DetailViewModel::DataItem*)items[0].m_pItem)->get()->GetInfo().GetField(MailHdrName_From).GetText(),
				((DetailViewModel::DataItem*)items[0].m_pItem)->get()->GetInfo().GetField(MailHdrName_Subj).GetText());
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

void MailMainView::AdjustMailSyncUiControls(MailMsgFileMgr::GrpProcStatus acc_busy_state)
{
	static auto start_icon = wxArtProvider::GetBitmap(wxASCII_STR("IcoToolRefresh"), wxASCII_STR(wxART_OTHER));
	static auto stop_icon = wxArtProvider::GetBitmap(wxASCII_STR("IcoToolStop"), wxASCII_STR(wxART_OTHER));

	auto tool_icon = acc_busy_state ? stop_icon : start_icon;
	auto tool_help = acc_busy_state ? ToolHlp_MailSyncStop : ToolHlp_MailSyncStart;
	int tool_id;

	if (!toolMailSyncProc->GetDropdownMenu()) { // The tool has no dropdown menu - replace the control
		// Replacing the tools by wxITEM_DROPDOWN type (RAD tools don't allow this type yet).
		// TODO: this wxITEM_DROPDOWN quick fix is supposed to be temporary, until fixed in the RAD editor
		auto pos = tlbrMaster->GetToolPos(toolMailSyncProc->GetId());
		tlbrMaster->DeleteTool(toolMailSyncProc->GetId());
		toolMailSyncProc = tlbrMaster->InsertTool(pos, wxID_ANY, wxEmptyString, tool_icon, wxNullBitmap,
			wxITEM_DROPDOWN, tool_help, wxEmptyString, nullptr);
		tool_id = toolMailSyncProc->GetId();
	} else {
		tool_id = toolMailSyncProc->GetId();
		tlbrMaster->SetToolNormalBitmap(tool_id, tool_icon);
		tlbrMaster->SetToolShortHelp(tool_id, tool_help);
	}

	auto tool_menu = new wxMenu();
	if (!acc_busy_state) {
		this->Unbind(wxEVT_COMMAND_TOOL_CLICKED, &MailMainView::toolStopSyncMail_OnToolClicked, this);
		this->Bind(wxEVT_COMMAND_TOOL_CLICKED, &MailMainView::toolStartSyncMail_OnToolClicked, this, tool_id);

		auto menu_item = tool_menu->Append(wxID_ANY, MnuLbl_MailSyncStartRecv);
		tool_menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &MailMainView::mnuMailSyncStartRecv_OnMenuSelection, this, menu_item->GetId());
		menu_item = tool_menu->Append(wxID_ANY, MnuLbl_MailSyncStartSend);
		tool_menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &MailMainView::mnuMailSyncStartSend_OnMenuSelection, this, menu_item->GetId());
	} else {
		this->Unbind(wxEVT_COMMAND_TOOL_CLICKED, &MailMainView::toolStartSyncMail_OnToolClicked, this);
		this->Bind(wxEVT_COMMAND_TOOL_CLICKED, &MailMainView::toolStopSyncMail_OnToolClicked, this, tool_id);

		auto menu_item = tool_menu->Append(wxID_ANY, MnuLbl_MailSyncStopRecv);
		tool_menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &MailMainView::mnuMailSyncStopRecv_OnMenuSelection, this, menu_item->GetId());
		menu_item->Enable(MailMsgFileMgr::GrpProcStatus::gpsProcReceiving & acc_busy_state);

		menu_item = tool_menu->Append(wxID_ANY, MnuLbl_MailSyncStopSend);
		tool_menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &MailMainView::mnuMailSyncStopSend_OnMenuSelection, this, menu_item->GetId());
		menu_item->Enable(MailMsgFileMgr::GrpProcStatus::gpsProcSending & acc_busy_state);
	}
	
	toolMailSyncProc->SetDropdownMenu(tool_menu); // Previous menu is deleted automatically if exists

	tlbrMaster->Realize();
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
	auto acc_busy_state =
		item ? GetAccItemBusyState(*item, msgFileMgr) : MailMsgFileMgr::GrpProcStatus::gpsNone;

	CallAfter([this, acc_busy_state]() { AdjustMailSyncUiControls(acc_busy_state); });
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

void MailMainView::StartMailSync(bool receiving, bool sending)
{
	wxBeginBusyCursor();
	const auto item = dvAccFolders->GetSelection();
	if (item.IsOk()) {
		auto data_item = (MasterViewModel::DataItem*)item.m_pItem;
		auto accounts = data_item->GetAccounts();
		for (auto& account : accounts) {
			msgFileMgr->InitGroup(account->Id, *account); // Refresh account info just in case
			if (receiving) msgFileMgr->StartMailRecv(account->Id);
			if (sending) msgFileMgr->StartMailSend(account->Id);
		}
		RefreshMasterToolsState(&item);
	}
	wxEndBusyCursor();
}

void MailMainView::StopMailSync(bool receiving, bool sending)
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
				if (sending) msgFileMgr->StopMailSend(account->Id);
				if (receiving) msgFileMgr->StopMailRecv(account->Id);
			}
			RefreshMasterToolsState(&item);
		}
	}
	wxEndBusyCursor();
}
