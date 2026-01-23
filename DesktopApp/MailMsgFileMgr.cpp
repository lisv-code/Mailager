#include "MailMsgFileMgr.h"
#include <algorithm>
#include <future>
#include <utility>
#include "../CoreAppLib/AppResCodes.h"
#include "../CoreAppLib/AccountCfg.h"
#include "../CoreAppLib/MailMsgReceiver.h"
#include "../CoreAppLib/MailMsgStore.h"
#include "../CoreAppLib/MailMsgTransmitter.h"
#include "AppCfg.h"

#define Log_Scope_Gen "MsgMgr"
#define Log_Scope_Rcv "MsgMgr.Rcv"
#define Log_Scope_Snd "MsgMgr.Snd"

namespace MailMsgFileMgr_Imp
{
	const bool StopReceiveOnMsgFileError = true;
	static bool is_mail_msg_to_send(const MailMsgFile* mail_msg);
	static bool is_mail_msg_status_needs_monitoring(const MailMsgFile* mail_msg);
}
using namespace MailMsgFileMgr_Imp;
using namespace LisLog;

MailMsgFileMgr::~MailMsgFileMgr()
{
	for (auto& item : mailGroups) { FreeGrpData(item.second); }
}

MailMsgFileMgr::GrpDataItem* MailMsgFileMgr::GetGrpData(GrpId grp_id)
{
	GrpDataItem* result = nullptr;
	// TODO: (?) multithreaded access lock
	const auto it = mailGroups.find(grp_id);
	if (it != mailGroups.end()) {
		result = (*it).second;
	}
	return result;
}

void MailMsgFileMgr::FreeGrpData(GrpDataItem* grp_data) { delete grp_data; }

int MailMsgFileMgr::InitGroup(GrpId grp_id, const AccountSettings& account)
{
	int result = 0;
	GrpDataItem* grp_item = GetGrpData(grp_id);
	if (!grp_item) {
		auto item = mailGroups.emplace(grp_id, new GrpDataItem());
		grp_item = (*item.first).second;
		result = ResCode_Created;
	}
	grp_item->MailAcc = account;
	return result;
}

MailMsgFileMgr::GrpProcStatus MailMsgFileMgr::GetProcStatus(GrpId grp_id)
{
	auto result = GrpProcStatus::gpsNone;
	if (taskMgr.GetTaskStatus(GetGrpTaskProcId(grp_id, true)) == LisThread::TaskProcStatus::tpsProcessing)
		result = (GrpProcStatus)(result | GrpProcStatus::gpsProcReceiving);
	if (taskMgr.GetTaskStatus(GetGrpTaskProcId(grp_id, false)) == LisThread::TaskProcStatus::tpsProcessing)
		result = (GrpProcStatus)(result | GrpProcStatus::gpsProcSending);
	return result;
}

bool MailMsgFileMgr::GetIter(GrpId grp_id, FilesIterator& begin, FilesIterator& end)
{
	auto grp_data = GetGrpData(grp_id);
	if (!grp_data)
		return false;

	begin = grp_data->MsgFiles.begin();
	end = grp_data->MsgFiles.end();
	return true;
}

int MailMsgFileMgr::LoadList(GrpId grp_id)
{
	auto grp_data = GetGrpData(grp_id);
	if (!grp_data) return Error_Gen_ItemNotFound;
	auto store_path = MailMsgStore::GetStoreDirPath(AppCfg.Get().AppDataDir.c_str(), grp_data->MailAcc.Directory.c_str());
	MailMsgStore mail_store;
	int res_code = mail_store.SetLocation(store_path.c_str(), grp_id);
	if (res_code >= 0) {
		auto files = mail_store.GetFileList();
		for (auto& file : files) {
			file.LoadInfo();
			auto file_grp_it = grp_data->MsgFiles.emplace(grp_data->MsgFiles.end(),
				std::make_shared<MailMsgFile>(std::move(file))); // TODO: multithreading warning - collection modification
			AfterMailMsgFileAdded(&(*file_grp_it), this, false);
		}
		logger->LogFmt(llInfo, Log_Scope_Gen " grp#%i Loaded mail message files: %i.",
			grp_id, files.size());
	} else
		logger->LogFmt(llError, Log_Scope_Gen " grp#%i Failed to load mail message files: %i.",
			grp_id, res_code);
	return res_code;
}

bool MailMsgFileMgr::StopMailRecv(GrpId grp_id)
{
	return taskMgr.StopTask(GetGrpTaskProcId(grp_id, true));
}

bool MailMsgFileMgr::StopMailSend(GrpId grp_id)
{
	return taskMgr.StopTask(GetGrpTaskProcId(grp_id, false));
}

bool MailMsgFileMgr::RemoveGroup(GrpId grp_id)
{
	auto grp_data = GetGrpData(grp_id);
	if (grp_data) {
		StopMailSend(grp_id);
		StopMailRecv(grp_id);
		FreeGrpData(grp_data);
		mailGroups.erase(grp_id);
		return true;
	}
	return false;
}

int MailMsgFileMgr::StartMailRecv(GrpId grp_id)
{
	auto grp_data = GetGrpData(grp_id);
	if (!grp_data) return Error_Gen_ItemNotFound;
	auto proc = std::bind(&MailMsgFileMgr::MailRecvProc, std::placeholders::_1, std::placeholders::_2);
	auto data = new MailSyncProcPrm{ this, grp_id };
	auto fin_callback = [grp_id, this](LisThread::TaskProcResult proc_result) {
		MailMsgFileMgr_EventData_SyncFinish sync_event_data(grp_id, proc_result);
		this->RaiseEvent(etRecvFinished, &sync_event_data);
		logger->LogFmt(llInfo, Log_Scope_Gen " acc#%i Mail recv finished, result: %i.", grp_id, proc_result);
	};
	logger->LogFmt(llInfo, Log_Scope_Gen " acc#%i Mail recv start...", grp_id);
	bool result = taskMgr.StartTask(GetGrpTaskProcId(grp_id, true), proc, data, fin_callback);
	if (!result) logger->LogFmt(llError, Log_Scope_Gen " acc#%i Failed to start mail recv.", grp_id);
	return result != false ? ResCode_Ok : Error_Gen_Undefined;
}

int MailMsgFileMgr::StartMailSend(GrpId grp_id)
{
	auto grp_data = GetGrpData(grp_id);
	if (!grp_data) return Error_Gen_ItemNotFound;
	auto proc = std::bind(&MailMsgFileMgr::MailSendProc, std::placeholders::_1, std::placeholders::_2);
	auto data = new MailSyncProcPrm{ this, grp_id };
	auto fin_callback = [grp_id, this](LisThread::TaskProcResult proc_result) {
		MailMsgFileMgr_EventData_SyncFinish sync_event_data(grp_id, proc_result);
		this->RaiseEvent(etSendFinished, &sync_event_data);
		logger->LogFmt(llInfo, Log_Scope_Gen " acc#%i Mail send finished, result: %i.", grp_id, proc_result);
	};
	logger->LogFmt(llInfo, Log_Scope_Gen " acc#%i Mail send start...", grp_id);
	bool result = taskMgr.StartTask(GetGrpTaskProcId(grp_id, false), proc, data, fin_callback);
	if (!result) logger->LogFmt(llError, Log_Scope_Gen " acc#%i Failed to start mail send.", grp_id);
	return result != false ? ResCode_Ok : Error_Gen_Undefined;
}

std::shared_ptr<MailMsgFile>& MailMsgFileMgr::CreateMailMsg(GrpId grp_id)
{
	auto msg_status = MailMsgStatus::mmsIsDraft;
	auto draft_it = draftMessages.emplace(draftMessages.end(),
		std::make_shared<MailMsgFile>(grp_id, msg_status));
	auto& result = *draft_it;
	auto msg_file_evt_handler = GetMailMsgFileEvtHandler(this);
	result->EventSubscribe(MailMsgFile_EventType::etDataSaving, msg_file_evt_handler);
	result->EventSubscribe(MailMsgFile_EventType::etDataSaved, msg_file_evt_handler);
	return result;
}

int MailMsgFileMgr::MailMsgFile_EventHandler(const MailMsgFile* mail_msg, const MailMsgFile::EventInfo& evt_info)
{
	int result = ResCode_Ok;
	if (MailMsgFile_EventType::etFileDeleted == evt_info.type) {
		// File deleted - Remove file from the group list
		auto grp_data = GetGrpData(mail_msg->GetGrpId());
		if (grp_data) {
			auto it = std::find_if(grp_data->MsgFiles.begin(), grp_data->MsgFiles.end(),
				[mail_msg](const auto& x) { return mail_msg == x.get(); });
			if (it != grp_data->MsgFiles.end()) grp_data->MsgFiles.erase(it); // TODO: multithreading warning - collection modification
		}
	} else if (MailMsgFile_EventType::etDataSaving == evt_info.type) {
		// File saving started - Generating file path
		std::basic_string<FILE_PATH_CHAR> *evt_prm = static_cast<MailMsgFile_EventData_DataSaving*>(evt_info.data);
		if (evt_prm) {
			auto acc = AccCfg.FindAccount(mail_msg->GetGrpId());
			if (acc) *evt_prm = MailMsgStore::GenerateFilePath(AppCfg.Get().AppDataDir.c_str(), acc->Directory.c_str());
			else result = Error_Gen_ItemNotFound; // Event parameter data is probably incorrect
		} else result = Error_Gen_Undefined;
	} else if (MailMsgFile_EventType::etDataSaved == evt_info.type) {
		// File saving finished - Move file from draft list to group
		auto draft_it = std::find_if(draftMessages.begin(), draftMessages.end(),
			[mail_msg](const auto& x) { return mail_msg == x.get(); });
		if (draft_it != draftMessages.end()) {
			auto grp_data = GetGrpData(mail_msg->GetGrpId());
			if (grp_data) {
				auto grp_it = grp_data->MsgFiles.emplace(grp_data->MsgFiles.end(), std::move(*draft_it)); // TODO: multithreading warning - collection modification
				draftMessages.erase(draft_it); // TODO: multithreading warning - collection modification
				AfterMailMsgFileAdded(&(*grp_it), this, true);
			}
		} else {
			// This can happen if the file has been handled already and moved to some group
		}
	} else if (MailMsgFile_EventType::etStatusChanged == evt_info.type) {
		// File status changed - Check if message is ready to be sent and enqueue it
		if (is_mail_msg_to_send(mail_msg)) {
			StartMailSend(mail_msg->GetGrpId());
		}
	}
	return result;
}

MailMsgFile::EventHandler MailMsgFileMgr::GetMailMsgFileEvtHandler(MailMsgFileMgr* mgr)
{
	// Possible optimization: cache the binded result by the instance pointer or use lambda
	return std::bind(&MailMsgFileMgr::MailMsgFile_EventHandler,
		mgr, std::placeholders::_1, std::placeholders::_2);
}

void MailMsgFileMgr::AfterMailMsgFileAdded(std::shared_ptr<MailMsgFile>* file, MailMsgFileMgr* mgr,
	bool raise_event)
{
	auto handler = GetMailMsgFileEvtHandler(mgr);
	file->get()->EventSubscribe(MailMsgFile_EventType::etFileDeleted, handler);
	if (is_mail_msg_status_needs_monitoring(file->get())) {
		file->get()->EventSubscribe(MailMsgFile_EventType::etStatusChanged, handler);
	}
	if (raise_event) {
		auto msg_event_data = static_cast<MailMsgFileMgr_EventData_NewMessage*>(file);
		mgr->RaiseEvent(etNewMessageAdded, msg_event_data);
	}
}

std::string MailMsgFileMgr::GetGrpTaskProcId(GrpId grp_id, bool is_receiving)
{
	return std::to_string(grp_id) + (is_receiving ? "rcv" : "snd");
}

LisThread::TaskProcResult MailMsgFileMgr::MailRecvProc(
	LisThread::TaskProcCtrl* proc_ctrl, LisThread::TaskWorkData work_data)
{
	auto prm = static_cast<MailSyncProcPrm*>(work_data);
	auto mgr = prm->Manager;
	GrpId grp_id = prm->GroupId;
	delete prm;

	auto mail_grp = mgr->GetGrpData(grp_id);
	if (!mail_grp) return Error_Gen_ItemNotFound;

	if (proc_ctrl->StopFlag) return Error_Gen_Operation_Interrupted;
	// ** Authenticating...
	std::string auth_data;
	int res_code = GetAuthData(auth_data, mail_grp->MailAcc.Incoming, proc_ctrl, mgr);
	if (res_code < 0) {
		mgr->logger->LogFmt(llError, Log_Scope_Rcv " grp#%i Authentication failed, can't receive mail: %i.",
			grp_id, res_code);
		return res_code;
	}

	if (proc_ctrl->StopFlag) return Error_Gen_Operation_Interrupted;
	// ** Receiving mail...
	int file_count = 0;
	MailMsgReceiver rcvr;
	res_code = rcvr.SetLocation(AppCfg.Get().TmpDataDir.c_str(), mail_grp->MailAcc.Incoming, grp_id);
	if (res_code >= 0) {
		auto mail_store_path = MailMsgStore::GetStoreDirPath(
			AppCfg.Get().AppDataDir.c_str(), mail_grp->MailAcc.Directory.c_str());
		MailMsgStore mail_store;
		mail_store.SetLocation(mail_store_path.c_str(), grp_id);
		auto grp_files = &mail_grp->MsgFiles;
		auto file_proc = [grp_id , &mail_store, grp_files, proc_ctrl, mgr, &file_count] (const FILE_PATH_CHAR* file_path)
		{
			int res_code;
			auto msg_file = mail_store.SaveMsgFile(file_path, true, !StopReceiveOnMsgFileError, res_code);
			if ((res_code >= 0) && (res_code = msg_file.LoadInfo() >= 0)) {
				auto file_grp_it = grp_files->emplace(grp_files->end(),
					std::make_shared<MailMsgFile>(std::move(msg_file))); // TODO: multithreading warning - collection modification
				++file_count;
				AfterMailMsgFileAdded(&(*file_grp_it), mgr, true);
			} else {
				mgr->logger->LogFmt(llError, Log_Scope_Rcv " grp#%i The mail message is broken, error: %i.",
					grp_id, res_code);
				if (StopReceiveOnMsgFileError) return false;
			}
			return !proc_ctrl->StopFlag;
		};
		res_code = rcvr.Receive(auth_data.c_str(), file_proc);
	}

	// ** Finishing execution...
	if (res_code >= 0) {
		mgr->logger->LogFmt(llInfo, Log_Scope_Rcv " grp#%i Mail messages received: %i.",
			grp_id, file_count);
		res_code = file_count;
	} else {
		mgr->logger->LogFmt(llError, Log_Scope_Rcv " grp#%i Couldn't receive mail messages, code: %i.",
			grp_id, res_code);
	}

	return res_code;
}

LisThread::TaskProcResult MailMsgFileMgr::MailSendProc(
	LisThread::TaskProcCtrl* proc_ctrl, LisThread::TaskWorkData work_data)
{
	auto prm = static_cast<MailSyncProcPrm*>(work_data);
	auto mgr = prm->Manager;
	GrpId grp_id = prm->GroupId;
	delete prm;

	auto mail_grp = mgr->GetGrpData(grp_id);
	if (!mail_grp) return Error_Gen_ItemNotFound;

	if (proc_ctrl->StopFlag) return Error_Gen_Operation_Interrupted;
	// ** Loading mail messages list...
	std::vector<std::shared_ptr<MailMsgFile>> mail_msg_list;
	MailMsgFileMgr::FilesIterator msg_list_begin, msg_list_end;
	if (!mgr->GetIter(grp_id, msg_list_begin, msg_list_end)) return Error_Gen_ItemNotFound;
	for (auto it = msg_list_begin; it != msg_list_end; ++it) {
		if (is_mail_msg_to_send(it->get())) mail_msg_list.push_back(*it);
	}
	if (mail_msg_list.empty()) return ResCode_NoContent;

	if (proc_ctrl->StopFlag) return Error_Gen_Operation_Interrupted;
	// ** Authenticating...
	std::string auth_data;
	int res_code = GetAuthData(auth_data, mail_grp->MailAcc.Outgoing, proc_ctrl, mgr);
	if (res_code < 0) {
		mgr->logger->LogFmt(llError, Log_Scope_Snd " grp#%i Authentication failed, can't send mail: %i.",
			grp_id, res_code);
		return res_code;
	}

	if (proc_ctrl->StopFlag) return Error_Gen_Operation_Interrupted;
	// ** Sending mail...
	int file_count = 0;
	MailMsgTransmitter trns;
	MailMsgTransmitter::TransmissionHandle trn_handle;
	res_code = trns.BeginTransmition(grp_id,
		mail_grp->MailAcc.Outgoing, auth_data.c_str(), mail_grp->MailAcc.GetMailbox(), trn_handle);
	if (res_code >= 0) {
		for (auto& mail_msg_item : mail_msg_list) {
			MailMsgFile* mail_msg_file = mail_msg_item.get();
			int send_res = trns.SendMailMessage(trn_handle, *mail_msg_file);
			if (send_res >= 0) {
				mail_msg_file->SetMailAsSent();
				mgr->logger->LogFmt(llInfo, Log_Scope_Snd " grp#%i Mail message has been sent: ", grp_id);
				++file_count;
			} else {
				// TODO: provide some name or id to the log
				mgr->logger->LogFmt(llError, Log_Scope_Snd " grp#%i Mail message send failed %i.", grp_id, send_res);
			}
			if (proc_ctrl->StopFlag) { res_code = Error_Gen_Operation_Interrupted;  break; }
		}
	}
	trns.EndTransmission(trn_handle);

	// ** Finishing execution...
	if (res_code >= 0) {
		mgr->logger->LogFmt(llInfo, Log_Scope_Snd " grp#%i Mail messages sent: %i.",
			grp_id, file_count);
		res_code = file_count;
	} else {
		mgr->logger->LogFmt(llError, Log_Scope_Snd " grp#%i Couldn't send mail messages, code: %i.",
			grp_id, res_code);
	}

	return res_code;
}

int MailMsgFileMgr::GetAuthData(std::string& auth_data, const Connections::ConnectionInfo& connection,
	LisThread::TaskProcCtrl* proc_ctrl, const MailMsgFileMgr* mgr)
{
	bool stop_func_reset = false;
	auto auth_event_handler = [proc_ctrl, &stop_func_reset, &mgr]
	(const Connections::ConnectionInfo& connection, ConnectionAuth::EventType evt_type, void* evt_data)
		{
			if (proc_ctrl->StopFlag) return false;
			switch (evt_type) {
			case ConnectionAuth::etPswdRequest:
				return AuthEvtProc_UserPswd(static_cast<ConnectionAuth::EventData_PswdRequest*>(evt_data), connection, mgr);
			case ConnectionAuth::etStopFunction:
			{
				stop_func_reset = AuthEvtProc_StopFunc(static_cast<ConnectionAuth::EventData_StopFunction*>(evt_data), proc_ctrl);
				return true; // allow execution (may begin showing some status)
			} }
			return false; // unknown auth type
		};

	ConnectionAuth auth(AppCfg.Get().AppDataDir.c_str(), connection);
	int result = auth.GetAuthData(auth_data, auth_event_handler);
	if (stop_func_reset) proc_ctrl->StopFunc = nullptr; // The auth call has finished, so the function is not valid anymore
	return result;
}

bool MailMsgFileMgr::AuthEvtProc_UserPswd(ConnectionAuth::EventData_PswdRequest* pswd_data,
	const Connections::ConnectionInfo& connection, const MailMsgFileMgr* mgr)
{
	int result = -1;
	if (mgr) {
		std::string pswd;
		bool need_save = false;
		MailMsgFileMgr_EventData_CredentialsRequest cred_event_data{ &connection, &pswd, &need_save };
		result = mgr->RaiseEvent(etCredentialsRequest, &cred_event_data);
		if (result >= 0) {
			pswd_data->PswdData = *cred_event_data.PswdData;
			pswd_data->NeedSave = *cred_event_data.NeedSave;
		}
	}
	return result >= 0;
}

bool MailMsgFileMgr::AuthEvtProc_StopFunc(ConnectionAuth::EventData_StopFunction* stop_func,
	LisThread::TaskProcCtrl* proc_ctrl)
{
	ConnectionAuth::EventData_StopFunction func1 = *stop_func;
	proc_ctrl->StopFunc = [func1]() { func1(); };
	return true;
}

bool MailMsgFileMgr_Imp::is_mail_msg_to_send(const MailMsgFile* mail_msg)
{
	return mail_msg->CheckStatusFlags(
		MailMsgStatus::mmsIsOutgoing,
		MailMsgStatus::mmsIsDraft | MailMsgStatus::mmsIsSent | MailMsgStatus::mmsIsDeleted);
}

bool MailMsgFileMgr_Imp::is_mail_msg_status_needs_monitoring(const MailMsgFile* mail_msg)
{
	return mail_msg->CheckStatusFlags(
		MailMsgStatus::mmsIsDraft | MailMsgStatus::mmsIsOutgoing,
		MailMsgStatus::mmsIsSent | MailMsgStatus::mmsIsDeleted);
}
