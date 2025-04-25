#include "MailMsgFileMgr.h"
#include <algorithm>
#include <future>
#include <utility>
#include <LisCommon/StrUtils.h>
#include "../CoreAppLib/AccountCfg.h"
#include "../CoreAppLib/MailMsgReceiver.h"
#include "../CoreAppLib/MailMsgStore.h"
#include "../CoreAppLib/MailMsgTransmitter.h"
#include "AppCfg.h"

#define Log_Scope "MsgFileMgr"

namespace MailMsgMgr_Imp
{
	const int ThreadRetCode_Interrupted = -1023;
	const int ThreadRetCode_Failure = -1024;
	const int FilePathInitError = -1;
}
using namespace MailMsgMgr_Imp;
using namespace LisLog;

MailMsgFileMgr::~MailMsgFileMgr()
{
	for (auto& item : grpData) { FreeGrpData(item.second); }
}

MailMsgFileMgr::GrpDataItem* MailMsgFileMgr::GetGrpData(GrpId grp_id, bool auto_create)
{
	GrpDataItem* result = nullptr;
	// TODO: (?) multithreaded access lock
	const auto it = grpData.find(grp_id);
	if (it == grpData.end()) {
		if (auto_create) {
			auto grp_data = new GrpDataItem;
			auto item = grpData.insert(std::make_pair(grp_id, grp_data));
			result = (*item.first).second;
		}
	} else {
		result = (*it).second;
	}
	return result;
}

void MailMsgFileMgr::FreeGrpData(GrpDataItem* grp_data) { delete grp_data; }

MailMsgFileMgr::MailMsgGrpStatus MailMsgFileMgr::GetProcStatus(GrpId grp_id)
{
	return LisThread::TaskProcStatus::tpsProcessing
			== taskMgr.GetTaskStatus(std::to_string(grp_id))
		? MailMsgGrpStatus::mgsProcessing : MailMsgGrpStatus::mgsNone;
}

bool MailMsgFileMgr::GetIter(GrpId grp_id, FilesIterator& begin, FilesIterator& end)
{
	// TODO: multithreaded access lock
	auto grp_data = GetGrpData(grp_id, false);
	if (!grp_data)
		return false;

	begin = grp_data->begin();
	end = grp_data->end();
	return true;
}

bool MailMsgFileMgr::LoadList(GrpId grp_id, const char* acc_dir)
{
	auto store_path = MailMsgStore::GetStoreDirPath(AppCfg.Get().AppDataDir.c_str(), acc_dir);
	MailMsgStore mail_store;
	int res_code = mail_store.SetLocation(store_path.c_str(), grp_id);
	if (res_code >= 0) {
		auto files = mail_store.GetFileList();
		auto event_handler = std::bind(&MailMsgFileMgr::MailMsgFile_EventHandler,
			this, std::placeholders::_1, std::placeholders::_2);
		auto grp_data = GetGrpData(grp_id, true);
		for (auto& file : files) {
			file.LoadInfo();
			file.EventSubscribe(MailMsgFile_EventType::etFileDeleted, event_handler);
			grp_data->push_back(std::make_shared<MailMsgFile>(file));
		}
		logger->LogFmt(llInfo, Log_Scope " grp#%i Loaded mail message files: %i.",
			grp_id, files.size());
	} else
		logger->LogFmt(llError, Log_Scope " grp#%i Failed to load mail message files: %i.",
			grp_id, res_code);
	return res_code >= 0;
}

bool MailMsgFileMgr::StopProcessing(GrpId grp_id)
{
	return taskMgr.StopTask(std::to_string(grp_id));
}

bool MailMsgFileMgr::RemoveGroup(GrpId grp_id)
{
	auto grp_data = GetGrpData(grp_id, false);
	if (grp_data) {
		StopProcessing(grp_id);
		FreeGrpData(grp_data);
		grpData.erase(grp_id);
		return true;
	}
	return false;
}

bool MailMsgFileMgr::StartMailSync(GrpId grp_id, const AccountSettings& account)
{
	auto grp_data = GetGrpData(grp_id, true); // To be sure that it exists
	if (!grp_data) return false; // Unlikely to happen, but potential failure if memory not enough
	auto proc = std::bind(&MailMsgFileMgr::MailReceiveProc, std::placeholders::_1, std::placeholders::_2);
	auto data = new MailSyncProcPrm{ this, account };
	auto result = taskMgr.StartTask(std::to_string(grp_id), proc, data);
	if (result)
		logger->LogFmt(llInfo, Log_Scope " acc#%i Mail sync started...", account.Id);
	return result;
}

std::shared_ptr<MailMsgFile>& MailMsgFileMgr::CreateMailMsg(GrpId grp_id)
{
	auto msg_status = (MailMsgStatus)(MailMsgStatus::mmsIsDraft);
	auto draft_it = draftMessages.emplace(draftMessages.end(),
		std::make_shared<MailMsgFile>(grp_id, nullptr, msg_status));
	auto& result = *draft_it;
	auto msg_file_evt_handler = std::bind(&MailMsgFileMgr::MailMsgFile_EventHandler,
		this, std::placeholders::_1, std::placeholders::_2);
	result->EventSubscribe(MailMsgFile_EventType::etDataSaving, msg_file_evt_handler);
	result->EventSubscribe(MailMsgFile_EventType::etDataSaved, msg_file_evt_handler);
	return result;
}

int MailMsgFileMgr::MailMsgFile_EventHandler(const MailMsgFile* mail_msg, const MailMsgFile::EventInfo& evt_info)
{
	if (MailMsgFile_EventType::etFileDeleted == evt_info.type) {
		// Remove file from the group list
		auto grp_data = GetGrpData(mail_msg->GetGrpId(), true);
		auto it = std::find_if(grp_data->begin(), grp_data->end(),
			[mail_msg](const auto& x) { return mail_msg  == x.get(); });
		if (it != grp_data->end()) grp_data->erase(it); // TODO: multithreading warning - collection modification
	} else if (MailMsgFile_EventType::etDataSaving == evt_info.type) {
		// Generating file path
		FILE_PATH_CHAR** evt_prm = static_cast<MailMsgFile_EventData_DataSaving*>(evt_info.data);
		auto acc = AccCfg.FindAccount(mail_msg->GetGrpId());
		if (!acc) return FilePathInitError;
		auto file_path = MailMsgStore::GenerateFilePath(AppCfg.Get().AppDataDir.c_str(), acc->Directory.c_str());
		*evt_prm = LisStr::StrCopy(file_path.c_str());
	} else if (MailMsgFile_EventType::etDataSaved == evt_info.type) {
		// Move file from draft list to group
		auto draft_it = std::find_if(draftMessages.begin(), draftMessages.end(),
			[mail_msg](const auto& x) { return mail_msg == x.get(); });
		if (draft_it != draftMessages.end()) {
			auto grp_data = GetGrpData(mail_msg->GetGrpId(), true);
			auto grp_it = grp_data->emplace(grp_data->end(), std::move(*draft_it)); // TODO: multithreading warning - collection modification
			draftMessages.erase(draft_it); // TODO: multithreading warning - collection modification
			AfterMailMsgFileAdded(this, &(*grp_it));
		} else {
			// This can happen if the file has been handled already and moved to some group
		}
	}
	return 0;
}

LisThread::TaskProcResult MailMsgFileMgr::MailReceiveProc(
	LisThread::TaskProcCtrl* proc_ctrl, LisThread::TaskWorkData work_data)
{
	auto prm = static_cast<MailSyncProcPrm*>(work_data);
	auto mgr = prm->Manager;
	AccountSettings acc(prm->Account);
	GrpId grp_id = acc.Id;
	delete prm;

	LisThread::TaskProcResult res_code = ThreadRetCode_Interrupted;

	std::string auth_data;
	if (!proc_ctrl->StopFlag) {
		// ** Authenticating...
		res_code = GetAuthData(auth_data, acc.Incoming, proc_ctrl, mgr);
		if (res_code < 0) {
			mgr->logger->LogFmt(llError, Log_Scope " grp#%i Authentication failed, can't receive mail: %i.",
				grp_id, res_code);
		}
	}

	int file_count = 0;
	if (!proc_ctrl->StopFlag && (res_code >= 0)) {
		// ** Receiving mail...
		MailMsgReceiver rcvr;
		res_code = rcvr.SetLocation(AppCfg.Get().TmpDataDir.c_str(), acc.Incoming, grp_id);
		if (res_code >= 0) {
			auto files = mgr->GetGrpData(grp_id, false);
			if (files) {
				auto mail_store_path = MailMsgStore::GetStoreDirPath(AppCfg.Get().AppDataDir.c_str(), acc.Directory.c_str());
				MailMsgStore mail_store;
				mail_store.SetLocation(mail_store_path.c_str(), grp_id);
				auto ret_proc = [&mail_store, files, proc_ctrl, mgr, &file_count]
					(const FILE_PATH_CHAR* file_path)
					{
						auto msg_file = mail_store.SaveMsgFile(file_path, true);
						if (msg_file.LoadInfo() >= 0) {
							auto files_it = files->emplace(files->end(), std::make_shared<MailMsgFile>(std::move(msg_file))); // TODO: multithreading warning - collection modification
							++file_count;
							AfterMailMsgFileAdded(mgr, &(*files_it));
						} else {
							// TODO: Handle the error (broken message file)
						}
						return !proc_ctrl->StopFlag ;
					};
				res_code = rcvr.Receive(auth_data.c_str(), ret_proc);
			} else {
				res_code = ThreadRetCode_Failure;
			}
		}
	}

	// ** Finishing execution...
	if (res_code >= 0) {
		mgr->logger->LogFmt(llInfo, Log_Scope " grp#%i Mail messages received: %i.",
			grp_id, file_count);
	} else {
		mgr->logger->LogFmt(llError, Log_Scope " grp#%i Failed to receive mail: %i.",
			grp_id, res_code);
	}
	MailMsgFileMgr_EventData_SyncFinish sync_event_data{ grp_id, static_cast<size_t>(file_count) };
	mgr->RaiseEvent(etSyncFinished, &sync_event_data);

	return res_code;
}

LisThread::TaskProcResult MailMsgFileMgr::MailSendProc(
	LisThread::TaskProcCtrl* proc_ctrl, LisThread::TaskWorkData work_data)
{
	auto prm = static_cast<MailSyncProcPrm*>(work_data);
	auto mgr = prm->Manager;
	AccountSettings acc(prm->Account);
	GrpId grp_id = acc.Id;
	delete prm;

	LisThread::TaskProcResult res_code = ThreadRetCode_Interrupted;

	std::vector<std::shared_ptr<MailMsgFile>> mail_msg_list;
	if (!proc_ctrl->StopFlag) {
		// ** Loading mail list...
		MailMsgFileMgr::FilesIterator msg_list_begin, msg_list_end;
		if (!mgr->GetIter(grp_id, msg_list_begin, msg_list_end)) {
			mgr->LoadList(grp_id, acc.Directory.c_str());
			mgr->GetIter(grp_id, msg_list_begin, msg_list_end);
		}
		for (auto it = msg_list_begin; it != msg_list_end; ++it) {
			auto msg = *it;
			auto status = msg->GetStatus();
			bool is_matched = (MailMsgStatus::mmsIsOutgoing & status)
				&& !(MailMsgStatus::mmsIsDraft & status) && !(MailMsgStatus::mmsIsSent & status);
			if (is_matched) mail_msg_list.push_back(msg);
		}
	}

	std::string auth_data;
	if (!proc_ctrl->StopFlag) {
		// ** Authenticating...
		res_code = GetAuthData(auth_data, acc.Outgoing, proc_ctrl, mgr);
		if (res_code < 0) {
			mgr->logger->LogFmt(llError, Log_Scope " grp#%i Authentication failed, can't send mail: %i.",
				grp_id, res_code);
		}
	}

	int file_count = 0;
	if (!proc_ctrl->StopFlag && (res_code >= 0)) {
		// ** Sending mail...
		MailMsgTransmitter trns;
		//res_code = trns.SetLocation(AppCfg.Get().TmpDataDir.c_str(), acc.Outgoing, grp_id);
		if (res_code >= 0) {
			auto files = mgr->GetGrpData(grp_id, false);
			// ...
		}
	}

	// ** Finishing execution...
	if (res_code >= 0) {
		mgr->logger->LogFmt(llInfo, Log_Scope " grp#%i Mail messages sent: %i.",
			grp_id, file_count);
	} else {
		mgr->logger->LogFmt(llError, Log_Scope " grp#%i Failed to send mail: %i.",
			grp_id, res_code);
	}
	MailMsgFileMgr_EventData_SyncFinish sync_event_data{ grp_id, static_cast<size_t>(file_count) };
	mgr->RaiseEvent(etSyncFinished, &sync_event_data);

	return res_code;
}

void MailMsgFileMgr::AfterMailMsgFileAdded(MailMsgFileMgr* mgr, std::shared_ptr<MailMsgFile>* file)
{
	file->get()->EventSubscribe(MailMsgFile_EventType::etFileDeleted,
		std::bind(&MailMsgFileMgr::MailMsgFile_EventHandler,
			mgr, std::placeholders::_1, std::placeholders::_2));

	auto msg_event_data = static_cast<MailMsgFileMgr_EventData_NewMessage*>(file);
	mgr->RaiseEvent(etNewMessageAdded, msg_event_data);
}

int MailMsgFileMgr::GetAuthData(std::string& auth_data, const Connections::ConnectionInfo& connection,
	LisThread::TaskProcCtrl* proc_ctrl, const MailMsgFileMgr* mgr)
{
	bool stop_func_reset = false;
	auto auth_event_proc = [proc_ctrl, &stop_func_reset, &mgr]
		(const Connections::ConnectionInfo& connection, ConnectionAuth::EventParams& params)
		{
			if (proc_ctrl->StopFlag) return false;
			if ((Connections::AuthenticationType::catUserPswd == connection.AuthType)
				&& (ConnectionAuth::etDataRequest == params.Type))
			{
				return AuthEvtProc_UserPswd(params, connection, mgr);
			}
			if (Connections::AuthenticationType::catOAuth2 == connection.AuthType) {
				stop_func_reset = AuthEvtProc_OAuth2(params, proc_ctrl);
				return true; // allow execution (may begin showing some status)
			}
			return false; // unknown auth type
		};

	ConnectionAuth auth(AppCfg.Get().AppDataDir.c_str(), connection);
	int result = auth.GetAuthData(auth_data, auth_event_proc);
	if (stop_func_reset) proc_ctrl->StopFunc = nullptr;
	return result;
}

bool MailMsgFileMgr::AuthEvtProc_UserPswd(ConnectionAuth::EventParams& event_params,
	const Connections::ConnectionInfo& connection, const MailMsgFileMgr* mgr)
{
	bool result = false;
	if ((ConnectionAuth::etDataRequest == event_params.Type) && mgr) {
		std::string pswd;
		bool need_save = false;
		MailMsgFileMgr_EventData_CredentialsRequest cred_event_data{ &connection, &pswd, &need_save };
		result = mgr->RaiseEvent(etNewMessageAdded, &cred_event_data);
		if (result <= 0) {
			event_params.StrData = *cred_event_data.PswdData;
			event_params.NeedSave = *cred_event_data.NeedSave;
		}
	}
	return result;
}

bool MailMsgFileMgr::AuthEvtProc_OAuth2(ConnectionAuth::EventParams& event_params,
	LisThread::TaskProcCtrl* proc_ctrl)
{
	bool result = false;
	if (ConnectionAuth::etStopFunction == event_params.Type) {
		result = true;
		auto stop_func = event_params.StopFunc;
		proc_ctrl->StopFunc = [stop_func]() { stop_func(); };
	}
	return result;
}
