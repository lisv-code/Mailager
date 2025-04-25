#include "MailMainView.h"
#include <chrono>
#include <thread>
#include "../../CoreAppLib/ConnectionInfo.h"
#include "MasterViewModel.h"
#include "DetailViewModel.h"
#include "../CredentialsWnd/CredentialsWnd.h"

namespace MailMainView_Def
{
	enum MailMsgProc_Event
	{
		evtCredentialsRequest,
		evtNewMessageAdded,
		evtSyncFinished
	};

	const auto EventWaitDuration = std::chrono::milliseconds(200);

	struct CredReqData {
		const Connections::ConnectionInfo* Connection;
		std::string* PswdData;
		bool* NeedSave;
	};
}
using namespace MailMainView_Def;

wxDEFINE_EVENT(MAIL_MSG_PROC_EVENT, wxCommandEvent);

void MailMainView::InitMailMsgProcEvent()
{
	isStop = false;
	Bind(MAIL_MSG_PROC_EVENT, &MailMainView::MailMsgCommandHandler, this);

	auto mail_mgr_evt_handler = std::bind(&MailMainView::MailMsgProcEventHandler,
		this, std::placeholders::_1, std::placeholders::_2);
	msgFileMgr->EventSubscribe(MailMsgFileMgr_EventType::etCredentialsRequest, mail_mgr_evt_handler);
	msgFileMgr->EventSubscribe(MailMsgFileMgr_EventType::etNewMessageAdded, mail_mgr_evt_handler);
	msgFileMgr->EventSubscribe(MailMsgFileMgr_EventType::etSyncFinished, mail_mgr_evt_handler);
}

void MailMainView::FreeMailMsgProcEvent()
{
	isStop = true;
}

int MailMainView::MailMsgProcEventHandler(const MailMsgFileMgr* mail_mgr, const MailMsgFileMgr::EventInfo& evt_info)
{
	// Check if need to wait, then sleep while main thread is busy and not cancelled
	bool needWait = NeedProcEventWait(evt_info.type, false);
	std::unique_lock<std::mutex> lock1(mutex1, std::defer_lock);
	while (needWait && !isStop && !lock1.try_lock())
		std::this_thread::sleep_for(EventWaitDuration);
	// Check if need to wait for result, prepare for result
	needWait = NeedProcEventWait(evt_info.type, true);
	procResult = 0;
	// Send (queue) the event to main thread and wait for result if needed
	if (RouteProcEvent(this, evt_info)) {
		while (needWait && !isStop && !procResult)
			std::this_thread::sleep_for(EventWaitDuration);
	}
	return procResult;
}

bool MailMainView::NeedProcEventWait(MailMsgFileMgr_EventType evt_type, bool event_finish)
{
	return evt_type == MailMsgFileMgr_EventType::etCredentialsRequest;
}

bool MailMainView::RouteProcEvent(wxEvtHandler* dest, const MailMsgFileMgr::EventInfo& evt_info)
{
	auto cmd_evt = new wxCommandEvent(MAIL_MSG_PROC_EVENT);
	switch (evt_info.type) {
	case MailMsgFileMgr_EventType::etCredentialsRequest:
	{
		cmd_evt->SetInt((int)MailMsgProc_Event::evtCredentialsRequest);
		auto cred_req = static_cast<MailMsgFileMgr_EventData_CredentialsRequest*>(evt_info.data);
		cmd_evt->SetClientData(new CredReqData{ cred_req->Connection, cred_req->PswdData, cred_req->NeedSave });
	}
	break;
	case MailMsgFileMgr_EventType::etNewMessageAdded:
	{
		cmd_evt->SetInt((int)MailMsgProc_Event::evtNewMessageAdded);
		auto new_msg = static_cast<MailMsgFileMgr_EventData_NewMessage*>(evt_info.data);
		cmd_evt->SetClientData(new_msg);
	}
	break;
	case MailMsgFileMgr_EventType::etSyncFinished:
		cmd_evt->SetInt((int)MailMsgProc_Event::evtSyncFinished);
		auto sync_fin = static_cast<MailMsgFileMgr_EventData_SyncFinish*>(evt_info.data);
		// No data actually is passed to further processing
		break;
	}
	wxQueueEvent(dest, cmd_evt);
	return true;
}

void MailMainView::MailMsgCommandHandler(wxCommandEvent& event)
{
	auto evt_type = (MailMsgProc_Event)event.GetInt();
	long acc_id = event.GetExtraLong();
	switch (evt_type) {
	case MailMsgProc_Event::evtCredentialsRequest:
	{
		auto data = (CredReqData*)event.GetClientData();
		MailMsgEvent_CredentialsRequest(data->Connection, data->PswdData, data->NeedSave);
		delete data;
	}
	break;
	case MailMsgProc_Event::evtNewMessageAdded:
		MailMsgEvent_NewMessageAdded((std::shared_ptr<MailMsgFile>*)event.GetClientData());
		break;
	case MailMsgProc_Event::evtSyncFinished:
		MailMsgEvent_SyncFinished();
		break;
	}
}

void MailMainView::MailMsgEvent_CredentialsRequest(
	const Connections::ConnectionInfo* connection, std::string* pswd_data, bool* need_save)
{
	wxBeginBusyCursor();
	CredentialsWnd wndDlg(this);
	wndDlg.SetData(connection->Server.c_str(), connection->UserName.c_str(),
		pswd_data->c_str(), false);
	if (wxID_OK == wndDlg.ShowModal()) {
		wndDlg.GetData(*pswd_data, need_save);
		procResult = 1;
	} else {
		procResult = -1;
	}
	wxEndBusyCursor();
}

void MailMainView::MailMsgEvent_NewMessageAdded(std::shared_ptr<MailMsgFile>* mail_msg)
{
	bool is_refresh_needed = false;
	const auto item = dvAccFolders->GetSelection();
	if (item.IsOk()) {
		auto data_item = (MasterViewModel::DataItem*)item.m_pItem;
		auto folder_id = data_item->GetFolderId();
		if (!(folder_id > 0) || IsFolderMatches(folder_id, mail_msg->get())) {
			auto accounts = data_item->GetAccounts();
			for (auto& item : accounts)
				if (item->Id == mail_msg->get()->GetGrpId()) { is_refresh_needed = true; break; }
		}
	}
	if (is_refresh_needed) {
		auto model = static_cast<DetailViewModel*>(dvMailMsgList->GetModel());
		model->AddItem(*mail_msg);
		ResetFolderMailCount(dvAccFolders, -1);
	}
}

void MailMainView::MailMsgEvent_SyncFinished()
{
	RefreshMasterToolsState();
}
