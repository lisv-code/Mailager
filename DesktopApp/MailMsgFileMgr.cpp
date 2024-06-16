#include "MailMsgFileMgr.h"
#include <algorithm>
#include <future>
#include <utility>
#include "../CoreAppLib/MailMsgReceiver.h"
#include "../CoreAppLib/MailMsgStore.h"
#include "AppCfg.h"

using namespace MailMsgFileMgr_Def;

#define Log_Scope "MsgFileMgr"

namespace MailMsgMgr_Imp
{
	const int ThreadRetCode_Interrupted = -1023;
	const int ThreadRetCode_Failure = -1024;
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

MailMsgGrpStatus MailMsgFileMgr::GetProcStatus(GrpId grp_id)
{
	return LisThread::TaskProcStatus::tpsProcessing
			== taskMgr.GetTaskStatus(std::to_string(grp_id))
		? MailMsgGrpStatus::mgsProcessing : MailMsgGrpStatus::mgsNone;
}

bool MailMsgFileMgr::GetIter(GrpId grp_id, FilesIterator& begin, FilesIterator& end)
{
	auto grp_data = GetGrpData(grp_id, false);
	if (!grp_data)
		return false;

	begin = grp_data->begin();
	end = grp_data->end();
	return true;
}

bool MailMsgFileMgr::LoadList(GrpId grp_id, const char* acc_dir)
{
	auto store_path = MailMsgStore::GetStorePath(AppCfg.Get().AppDataDir.c_str(), acc_dir);
	MailMsgStore mail_store;
	int res_code = mail_store.SetLocation(store_path.c_str(), grp_id);
	if (res_code >= 0) {
		auto files = mail_store.GetFileList();
		auto event_handler = std::bind(&MailMsgFileMgr::MailMsgFile_EventHandler,
			this, std::placeholders::_1, std::placeholders::_2);
		auto grp_data = GetGrpData(grp_id, true);
		for (auto& file : files) {
			file.EventSubscribe(MailMsgFile_Def::etFileDeleted, event_handler);
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

bool MailMsgFileMgr::StartSyncMail(GrpId grp_id, const AccountSettings& account, ProcEventHandler callback)
{
	auto grp_data = GetGrpData(grp_id, true); // To be sure that it exists
	if (!grp_data) return false; // Unlikely to happen, but potential failure if memory not enough
	auto proc = std::bind(&MailMsgFileMgr::MailRetrieveProc, std::placeholders::_1, std::placeholders::_2);
	auto data = new MailSyncProcPrm{ this, grp_id, account, callback };
	auto result = taskMgr.StartTask(std::to_string(grp_id), proc, data);
	if (result)
		logger->LogFmt(llInfo, Log_Scope " acc#%i Mail sync started...", account.Id);
	return result;
}

int MailMsgFileMgr::MailMsgFile_EventHandler(const MailMsgFile* mail_msg, const MailMsgFile::EventInfo& evt_info)
{
	if (MailMsgFile_Def::etFileDeleted == evt_info.type) {
		for (auto& item : grpData) {
			auto& list = *item.second;
			auto it = std::find_if(list.begin(), list.end(),
				[mail_msg](const auto& x) { return mail_msg  == x.get(); });
			if (it != list.end()) {
				list.erase(it);
				break;
			}
		}
	}
	return 0;
}

LisThread::TaskProcResult MailMsgFileMgr::MailRetrieveProc(
	LisThread::TaskProcCtrl* proc_ctrl, LisThread::TaskWorkData work_data)
{
	auto prm = static_cast<MailSyncProcPrm*>(work_data);
	auto mgr = prm->Manager;
	GrpId grp_id = prm->GroupId;
	AccountSettings acc(prm->Account);
	ProcEventHandler clbk(prm->Callback);
	delete prm;

	LisThread::TaskProcResult res_code = ThreadRetCode_Interrupted;

	std::string auth_data;
	if (!proc_ctrl->StopFlag) {
		// ** Authenticating...
		res_code = GetAuthData(auth_data, grp_id, acc.Incoming, proc_ctrl, clbk);
		if (res_code < 0) {
			mgr->logger->LogFmt(llError, Log_Scope " grp#%i Authentication failed, can't retrieve mail: %i.",
				grp_id, res_code);
		}
	}

	int file_count = 0;
	if (!proc_ctrl->StopFlag && (res_code >= 0)) {
		// ** Receiving mail...
		MailMsgReceiver rcvr;
		res_code = rcvr.SetLocation(AppCfg.Get().TmpDataDir.c_str(), AppCfg.Get().AppDataDir.c_str(),
			acc.Directory.c_str(), acc.Incoming, grp_id);
		if (res_code >= 0) {
			auto files = mgr->GetGrpData(grp_id, false);
			if (files) {
				auto ret_proc = [files, proc_ctrl, mgr, &file_count, grp_id, &clbk]
					(MailMsgFile& file)
					{
						files->push_back(std::make_shared<MailMsgFile>(file));
						++file_count;
						AfterMailMsgFileAdded(mgr, grp_id, &files->back(), clbk);
						return !proc_ctrl->StopFlag;
					};
				res_code = rcvr.Retrieve(auth_data.c_str(), ret_proc);
			} else {
				res_code = ThreadRetCode_Failure;
			}
		}
	}

	// ** Finishing execution...
	if (res_code >= 0) {
		mgr->logger->LogFmt(llInfo, Log_Scope " grp#%i Mail messages retrieved: %i.",
			grp_id, file_count);
	}
	else {
		mgr->logger->LogFmt(llError, Log_Scope " grp#%i Failed to retrieve mail: %i.",
			grp_id, res_code);
	}
	ProcEventData data;
	data.Type = ProcEventType::petSyncFinished;
	data.ItemsCount = file_count;
	clbk(grp_id, data);

	return res_code;
}

void MailMsgFileMgr::AfterMailMsgFileAdded(MailMsgFileMgr* mgr, GrpId grp_id, std::shared_ptr<MailMsgFile>* file,
	const ProcEventHandler& callback)
{
	file->get()->EventSubscribe(MailMsgFile_Def::etFileDeleted,
		std::bind(&MailMsgFileMgr::MailMsgFile_EventHandler,
			mgr, std::placeholders::_1, std::placeholders::_2));
	if (callback) {
		ProcEventData data;
		data.Type = ProcEventType::petNewMessageAdded;
		data.MailMsg = file;
		callback(grp_id, data);
	}
}

int MailMsgFileMgr::GetAuthData(std::string& auth_data,
	GrpId grp_id, const Connections::ConnectionInfo& connection,
	LisThread::TaskProcCtrl* proc_ctrl, const ProcEventHandler& callback)
{
	bool stop_func_reset = false;
	auto auth_event_proc = [proc_ctrl, &stop_func_reset, grp_id, &callback]
		(const Connections::ConnectionInfo& connection, ConnectionAuth::EventParams& params)
		{
			if (proc_ctrl->StopFlag) return false;
			if ((Connections::AuthenticationType::catUserPswd == connection.AuthType)
				&& (ConnectionAuth::etDataRequest == params.Type))
			{
				return AuthEvtProc_UserPswd(params, grp_id, connection, callback);
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
	GrpId grp_id, const Connections::ConnectionInfo& connection, const ProcEventHandler& callback)
{
	bool result = false;
	if ((ConnectionAuth::etDataRequest == event_params.Type) && callback) {
		std::string pswd;
		bool need_save = false;
		ProcEventData data;
		data.Type = ProcEventType::petCredentialsRequest;
		data.CredReq.Connection = &connection;
		data.CredReq.PswdData = &pswd;
		data.CredReq.NeedSave = &need_save;
		result = callback(grp_id, data);
		if (result) {
			event_params.StrData = *data.CredReq.PswdData;
			event_params.NeedSave = *data.CredReq.NeedSave;
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
