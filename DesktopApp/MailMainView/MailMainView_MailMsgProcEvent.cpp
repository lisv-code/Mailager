#include "MailMainView.h"
#include <chrono>
#include <thread>
#include "../../CoreAppLib/ConnectionInfo.h"
#include "MasterViewModel.h"
#include "DetailViewModel.h"
#include "../AppCfg.h"
#include "../CredentialsWnd/CredentialsWnd.h"

wxDEFINE_EVENT(MAIL_PROC_EVENT, wxCommandEvent);

void MailMainView::InitMailMsgProcEvent()
{
	Bind(MAIL_PROC_EVENT, &MailMainView::MailMsgCommandHandler, this);
	mailEvtRtr.Init(MAIL_PROC_EVENT, this, &msgFileMgr->MsgRegistry, &msgFileMgr->SyncMgr);
}

void MailMainView::FreeMailMsgProcEvent()
{
	mailEvtRtr.Init(MAIL_PROC_EVENT, nullptr, nullptr, nullptr);
}

void MailMainView::MailMsgCommandHandler(wxCommandEvent& event)
{
	auto evt_type = static_cast<MailEventRouter_Def::MailRoutedEvent>(event.GetInt());
	long acc_id = event.GetExtraLong();
	switch (evt_type) {
	case MailEventRouter_Def::MailRoutedEvent::CredentialsRequest:
	{
		auto data = (MailEventRouter_Def::CredReqData*)event.GetClientData();
		MailMsgEvent_CredentialsRequest(data->Connection, data->PswdData, data->NeedSave);
		delete data;
	}
	break;
	case MailEventRouter_Def::MailRoutedEvent::NewMessageAdded:
	{
		auto evt_data = static_cast<MailEventRouter_Def::NewMsgFileData*>(event.GetClientObject());
		MailMsgEvent_NewMessageAdded(evt_data->File);
	}
	break;
	case MailEventRouter_Def::MailRoutedEvent::SyncFinished:
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
		mailEvtRtr.SetProcResult(1); // Credentials have been returned
	} else {
		mailEvtRtr.SetProcResult(-1);
	}
	wxEndBusyCursor();
}

void MailMainView::MailMsgEvent_NewMessageAdded(std::shared_ptr<MailMsgFile>& mail_msg)
{
	bool is_refresh_needed = false;
	const auto item = dvAccFolders->GetSelection();
	if (item.IsOk()) {
		auto data_item = (MasterViewModel::DataItem*)item.m_pItem;
		auto folder_id = data_item->GetFolderId();
		if (!(folder_id > 0) || IsFolderMatches(folder_id, mail_msg.get())) {
			auto accounts = data_item->GetAccounts();
			for (auto& item : accounts)
				if (item->Id == mail_msg->GetGrpId()) { is_refresh_needed = true; break; }
		}
	}
	if (is_refresh_needed) {
		auto model = static_cast<DetailViewModel*>(dvMailMsgList->GetModel());
		model->AddItem(mail_msg);
		ResetFolderMailCount(dvAccFolders, -1);
	}
}

void MailMainView::MailMsgEvent_SyncFinished()
{
	RefreshMasterToolsState();
}
