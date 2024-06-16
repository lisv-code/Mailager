#include "MailMainView.h"
#include <chrono>
#include <thread>
#include "../../CoreAppLib/ConnectionInfo.h"
#include "MasterViewModel.h"
#include "DetailViewModel.h"
#include "../CredentialsWnd/CredentialsWnd.h"

namespace MailMainView_Def
{
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
}

void MailMainView::FreeMailMsgProcEvent()
{
	isStop = true;
}

bool MailMainView::MailMsgProcEventHandler(int acc_id, MailMsgFileMgr::ProcEventData evt_data)
{
	// Check if need to wait, then sleep while main thread is busy and not cancelled
	bool needWait = NeedProcEventWait(evt_data, false);
	std::unique_lock<std::mutex> lock1(mutex1, std::defer_lock);
	while (needWait && !isStop && !lock1.try_lock())
		std::this_thread::sleep_for(EventWaitDuration);
	// Check if need to wait for result, prepare for result
	needWait = NeedProcEventWait(evt_data, true);
	procResult = 0;
	// Send (queue) the event to main thread and wait for result if needed
	bool result = RouteProcEvent(this, acc_id, evt_data);
	if (result) {
		while (needWait && !isStop && !procResult)
			std::this_thread::sleep_for(EventWaitDuration);
		if (needWait) {
			result = procResult > 0;
		}
	}
	return result;
}

bool MailMainView::NeedProcEventWait(MailMsgFileMgr::ProcEventData evt_data, bool event_finish)
{
	return evt_data.Type == MailMsgFileMgr::ProcEventType::petCredentialsRequest;
}

bool MailMainView::RouteProcEvent(wxEvtHandler* dest, int acc_id, MailMsgFileMgr::ProcEventData evt_data)
{
	auto cmd_evt = new wxCommandEvent(MAIL_MSG_PROC_EVENT);
	cmd_evt->SetInt((int)evt_data.Type);
	cmd_evt->SetExtraLong(acc_id);
	switch (evt_data.Type) {
	case MailMsgFileMgr::ProcEventType::petCredentialsRequest:
		cmd_evt->SetClientData(new CredReqData {
			evt_data.CredReq.Connection,
			evt_data.CredReq.PswdData,
			evt_data.CredReq.NeedSave
		});
		break;
	case MailMsgFileMgr::ProcEventType::petNewMessageAdded:
		cmd_evt->SetClientData(evt_data.MailMsg);
		break;
	}
	wxQueueEvent(dest, cmd_evt);
	return true;
}

void MailMainView::MailMsgCommandHandler(wxCommandEvent& event)
{
	auto evt_type = (MailMsgFileMgr::ProcEventType)event.GetInt();
	long acc_id = event.GetExtraLong();
	switch (evt_type) {
	case MailMsgFileMgr::ProcEventType::petCredentialsRequest:
		{
			auto data = (CredReqData*)event.GetClientData();
			MailMsgEvent_CredentialsRequest(data->Connection, data->PswdData, data->NeedSave);
			delete data;
		}
		break;
	case MailMsgFileMgr::ProcEventType::petNewMessageAdded:
		MailMsgEvent_NewMessageAdded(acc_id, (std::shared_ptr<MailMsgFile>*)event.GetClientData());
		break;
	case MailMsgFileMgr::ProcEventType::petSyncFinished:
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

void MailMainView::MailMsgEvent_NewMessageAdded(int acc_id, std::shared_ptr<MailMsgFile>* mail_msg)
{
	bool is_refresh_needed = false;
	const auto item = dvAccFolders->GetSelection();
	if (item.IsOk()) {
		auto data_item = (MasterViewModel::DataItem*)item.m_pItem;
		auto folder_id = data_item->GetFolderId();
		if (!(folder_id > 0) || IsFolderMatches(folder_id, mail_msg->get())) {
			auto accounts = data_item->GetAccounts();
			for (auto& item : accounts)
				if (item->Id == acc_id) { is_refresh_needed = true; break; }
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
