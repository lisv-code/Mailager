#pragma once
#include <mutex>
#include <string>
#include <wx/event.h>
#include "../../CoreAppLib/MailSyncMgr.h"
#include "../MailMsgRegistry.h"

namespace MailEventRouter_Def
{
	enum class MailRoutedEvent {
		CredentialsRequest,
		NewMessageAdded,
		SyncFinished
	};

	struct CredReqData {
		const Connections::ConnectionInfo* Connection;
		std::string* PswdData;
		bool* NeedSave;
	};

	class NewMsgFileData : public wxClientData {
	public:
		std::shared_ptr<MailMsgFile> File;
		NewMsgFileData(const std::shared_ptr<MailMsgFile>& file) : File(file) { }
	};
}

class MailEventRouter
{
public:
	~MailEventRouter();
	void Init(wxEventType evt_type, wxEvtHandler* evt_handler,
		MailMsgRegistry* msg_registry, MailSyncMgr* sync_manager);
	void SetProcResult(int value);

private:
	wxEventType evtType = 0;
	wxEvtHandler* evtHandler = nullptr;

	bool isStop = false;
	std::mutex mutex1;
	int procResult;

	int AuthEventHandler(const ConnAuthFacade* mail_mgr, const ConnAuthFacade::EventInfo& evt_info);
	int MsgRegEventHandler(const MailMsgRegistry* mail_mgr, const MailMsgRegistry::EventInfo& evt_info);
	int SyncEventHandler(const MailSyncMgr* mail_mgr, const MailSyncMgr::EventInfo& evt_info);

	bool RouteAuthEvent(const ConnAuthFacade::EventInfo& evt_info);
	bool RouteMsgRegEvent(const MailMsgRegistry::EventInfo& evt_info);
	bool RouteSyncEvent(const MailSyncMgr::EventInfo& evt_info);
};
