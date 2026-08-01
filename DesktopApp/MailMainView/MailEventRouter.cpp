#include "MailEventRouter.h"
#include <chrono>
#include <thread>

using namespace MailEventRouter_Def;
namespace MailEventRouter_Imp
{
	const auto EventWaitDuration = std::chrono::milliseconds(200);
	static bool need_event_proc_wait(ConnAuthFacade_EventType evt_type, bool event_finish);
}
using namespace MailEventRouter_Imp;

MailEventRouter::~MailEventRouter()
{
	isStop = true;
}

void MailEventRouter::Init(wxEventType evt_type, wxEvtHandler* evt_handler,
	MailMsgRegistry* msg_registry, MailSyncMgr* sync_manager)
{
	evtType = evt_type;
	evtHandler = evt_handler;
	if (!evtHandler) return;

	if (msg_registry) {
		auto reg_evt_handler = std::bind(&MailEventRouter::MsgRegEventHandler,
			this, std::placeholders::_1, std::placeholders::_2);
		msg_registry->EventSubscribe(MailMsgRegistry_EventType::NewMsgFile, reg_evt_handler);
	}

	if (sync_manager) {
		auto sync_evt_handler = std::bind(&MailEventRouter::SyncEventHandler,
			this, std::placeholders::_1, std::placeholders::_2);
		sync_manager->EventSubscribe(MailSyncMgr_EventType::RecvFinished, sync_evt_handler);
		sync_manager->EventSubscribe(MailSyncMgr_EventType::SendFinished, sync_evt_handler);

		auto& auth_facade = sync_manager->AuthFacade;
		auto auth_evt_handler = std::bind(&MailEventRouter::AuthEventHandler,
			this, std::placeholders::_1, std::placeholders::_2);
		auth_facade.EventSubscribe(ConnAuthFacade_EventType::CredentialsRequest, auth_evt_handler);
	}
}

void MailEventRouter::SetProcResult(int value)
{
	procResult = value;
}

int MailEventRouter::AuthEventHandler(const ConnAuthFacade* mail_mgr, const ConnAuthFacade::EventInfo& evt_info)
{
	// Check if need to wait, then sleep while main thread is busy and not cancelled
	bool needWait = need_event_proc_wait(evt_info.type, false);
	std::unique_lock<std::mutex> lock1(mutex1, std::defer_lock);
	while (needWait && !isStop && !lock1.try_lock())
		std::this_thread::sleep_for(EventWaitDuration);
	// Check if need to wait for result, prepare for result
	needWait = need_event_proc_wait(evt_info.type, true);
	procResult = 0;
	// Send (queue) the event to main thread and wait for result if needed
	if (RouteAuthEvent(evt_info)) {
		while (needWait && !isStop && !procResult)
			std::this_thread::sleep_for(EventWaitDuration);
	}
	return procResult;
}

int MailEventRouter::MsgRegEventHandler(const MailMsgRegistry* mail_mgr, const MailMsgRegistry::EventInfo& evt_info)
{
	RouteMsgRegEvent(evt_info);
	return 0;
}

int MailEventRouter::SyncEventHandler(const MailSyncMgr* mail_mgr, const MailSyncMgr::EventInfo& evt_info)
{
	RouteSyncEvent(evt_info);
	return 0;
}

bool MailEventRouter::RouteAuthEvent(const ConnAuthFacade::EventInfo& evt_info)
{
	if (!evtHandler) return false;

	auto cmd_evt = new wxCommandEvent(evtType);
	switch (evt_info.type) {
	case ConnAuthFacade_EventType::CredentialsRequest:
	{
		cmd_evt->SetInt(static_cast<int>(MailRoutedEvent::CredentialsRequest));
		auto& auth_info = static_cast<const ConnAuthFacade_EventData&>(evt_info.data);
		cmd_evt->SetClientData(new CredReqData{ auth_info.Connection, auth_info.PswdData, auth_info.NeedSave });
	}
	break;
	default:
		return false; // ERROR: Unsupported event
	}
	wxQueueEvent(evtHandler, cmd_evt);
	return true;
}

bool MailEventRouter::RouteMsgRegEvent(const MailMsgRegistry::EventInfo& evt_info)
{
	if (!evtHandler) return false;

	auto cmd_evt = new wxCommandEvent(evtType);
	switch (evt_info.type) {
	case MailMsgRegistry_EventType::NewMsgFile:
	{
		cmd_evt->SetInt(static_cast<int>(MailRoutedEvent::NewMessageAdded));
		cmd_evt->SetClientObject(new NewMsgFileData(evt_info.data));
	}
	break;
	default:
		return false; // ERROR: Unsupported event
	}
	wxQueueEvent(evtHandler, cmd_evt);
	return true;
}

bool MailEventRouter::RouteSyncEvent(const MailSyncMgr::EventInfo& evt_info)
{
	if (!evtHandler) return false;

	auto cmd_evt = new wxCommandEvent(evtType);
	switch (evt_info.type) {
	case MailSyncMgr_EventType::RecvFinished:
	case MailSyncMgr_EventType::SendFinished:
	{
		cmd_evt->SetInt(static_cast<int>(MailRoutedEvent::SyncFinished));
		const auto& sync_fin = static_cast<const MailSyncMgr_EventData&>(evt_info.data);
		// No data is passed currently to further processing
	}
	break;
	default:
		return false; // ERROR: Unsupported event
	}
	wxQueueEvent(evtHandler, cmd_evt);
	return true;
}

bool MailEventRouter_Imp::need_event_proc_wait(ConnAuthFacade_EventType evt_type, bool event_finish)
{
	return evt_type == ConnAuthFacade_EventType::CredentialsRequest;
}
