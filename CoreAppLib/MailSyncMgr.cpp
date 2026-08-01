#include "MailSyncMgr.h"
#include <functional>
#include "../CoreAppLib/AppResCodes.h"
#include "../CoreAppLib/AccountConfig.h"
#include "../CoreAppLib/MailMsgReceiver.h"
#include "../CoreAppLib/MailMsgStore.h"
#include "../CoreAppLib/MailMsgTransmitter.h"

#define Log_Scope "MailSync"

namespace MailSyncMgr_Imp
{
	const bool StopReceiveOnMsgFileError = true;
}
using namespace MailSyncMgr_Imp;
using namespace LisLog;

MailSyncProcStatus MailSyncMgr::GetProcStatus(GrpId grp_id)
{
	auto result = MailSyncProcStatus::spsNone;
	if (taskMgr.GetTaskStatus(GetGrpTaskProcId(grp_id, true)) == LisThread::TaskProcStatus::tpsProcessing)
		result = result | MailSyncProcStatus::spsReceiving;
	if (taskMgr.GetTaskStatus(GetGrpTaskProcId(grp_id, false)) == LisThread::TaskProcStatus::tpsProcessing)
		result = result | MailSyncProcStatus::spsSending;
	return result;
}

int MailSyncMgr::StartMailRecv(GrpId grp_id, FileConsumer file_consumer)
{
	auto proc = std::bind(&MailSyncMgr::MailRecvProc, std::placeholders::_1, std::placeholders::_2);
	auto data = new MailSyncProcRecvParam{ { logger, grp_id, AuthFacade },
		TempDataDir, UserDataDir, file_consumer }; // Must be disposed by the task processor
	auto fin_callback = [grp_id, this](LisThread::TaskProcResult proc_result) {
		MailSyncMgr_EventData evt_prm(grp_id, proc_result);
		this->RaiseEvent(MailSyncMgr_EventType::RecvFinished, evt_prm);
		logger->LogFmt(llInfo, Log_Scope " acc#%i Mail recv finished, result: %i.", grp_id, proc_result);
	};
	logger->LogFmt(llInfo, Log_Scope " acc#%i Mail recv start...", grp_id);
	bool result = taskMgr.StartTask(GetGrpTaskProcId(grp_id, true), proc, data, fin_callback);
	if (!result) logger->LogFmt(llError, Log_Scope " acc#%i Failed to start mail recv.", grp_id);
	return result != false ? ResCode_Ok : Error_Gen_Undefined;
}

int MailSyncMgr::StartMailSend(GrpId grp_id, FileProvider file_provider)
{
	auto proc = std::bind(&MailSyncMgr::MailSendProc, std::placeholders::_1, std::placeholders::_2);
	auto data = new MailSyncProcSendParam{ { logger, grp_id, AuthFacade }, file_provider }; // Must be disposed by the task processor
	auto fin_callback = [grp_id, this](LisThread::TaskProcResult proc_result) {
		MailSyncMgr_EventData evt_prm(grp_id, proc_result);
		this->RaiseEvent(MailSyncMgr_EventType::SendFinished, evt_prm);
		logger->LogFmt(llInfo, Log_Scope " acc#%i Mail send finished, result: %i.", grp_id, proc_result);
	};
	logger->LogFmt(llInfo, Log_Scope " acc#%i Mail send start...", grp_id);
	bool result = taskMgr.StartTask(GetGrpTaskProcId(grp_id, false), proc, data, fin_callback);
	if (!result) logger->LogFmt(llError, Log_Scope " acc#%i Failed to start mail send.", grp_id);
	return result != false ? ResCode_Ok : Error_Gen_Undefined;
}

bool MailSyncMgr::StopMailRecv(GrpId grp_id)
{
	return taskMgr.StopTask(GetGrpTaskProcId(grp_id, true));
}

bool MailSyncMgr::StopMailSend(GrpId grp_id)
{
	return taskMgr.StopTask(GetGrpTaskProcId(grp_id, false));
}

LisThread::TaskProcResult MailSyncMgr::MailRecvProc(
	LisThread::TaskProcCtrl* proc_ctrl, LisThread::TaskWorkData work_data)
{
	auto prm = static_cast<MailSyncProcRecvParam*>(work_data);
	auto logger = prm->general.Logger;
	GrpId grp_id = prm->general.GroupId;
	auto& auth_facade = prm->general.AuthFacade;
	auto tmp_data_dir = prm->TmpDataDir;
	auto usr_data_dir = prm->UsrDataDir;
	auto file_consumer = prm->FileConsumer;
	delete prm;

	auto acc_ref = AccCfg.FindAccount(grp_id);
	if (!acc_ref) return Error_Gen_ItemNotFound;
	AccountSettings acc(*acc_ref);

	if (proc_ctrl->StopFlag) return Error_Gen_Operation_Interrupted;
	// ** Authenticating...
	std::string auth_data;
	int res_code = auth_facade.GetAuthData(auth_data, acc.Incoming, proc_ctrl);
	if (res_code < 0) {
		logger->LogFmt(llError, Log_Scope " grp#%i Authentication failed, can't receive mail: %i.", grp_id, res_code);
		return res_code;
	}

	if (proc_ctrl->StopFlag) return Error_Gen_Operation_Interrupted;
	// ** Receiving mail...
	int file_count = 0;
	MailMsgReceiver rcvr;
	res_code = rcvr.SetLocation(tmp_data_dir.c_str(), acc.Incoming, grp_id);
	if (res_code >= 0) {
		auto mail_store_path = MailMsgStore::GetStoreDirPath(usr_data_dir.c_str(), acc.Directory.c_str());
		MailMsgStore mail_store;
		mail_store.SetLocation(mail_store_path.c_str(), grp_id);
		auto file_proc = [grp_id, &mail_store, file_consumer, proc_ctrl, logger, &file_count](const FILE_PATH_CHAR* file_path)
		{
			int res_code;
			auto msg_file = mail_store.SaveMsgFile(file_path, true, !StopReceiveOnMsgFileError, res_code);
			if ((res_code _Is_Ok_ResCode)
				&& (res_code = msg_file.LoadInfo() _Is_Ok_ResCode) && (res_code = file_consumer(msg_file) _Is_Ok_ResCode))
			{
				++file_count;
			} else {
				logger->LogFmt(llError, Log_Scope " grp#%i The mail message is broken, error: %i.", grp_id, res_code);
				if (StopReceiveOnMsgFileError) return false;
			}
			return !proc_ctrl->StopFlag;
		};
		res_code = rcvr.Receive(auth_data.c_str(), file_proc);
	}

	// ** Finishing execution...
	if (res_code >= 0) {
		logger->LogFmt(llInfo, Log_Scope " grp#%i Mail messages received: %i.", grp_id, file_count);
		res_code = file_count;
	} else {
		logger->LogFmt(llError, Log_Scope " grp#%i Couldn't receive mail messages, code: %i.", grp_id, res_code);
	}

	return res_code;
}

LisThread::TaskProcResult MailSyncMgr::MailSendProc(
	LisThread::TaskProcCtrl* proc_ctrl, LisThread::TaskWorkData work_data)
{
	auto prm = static_cast<MailSyncProcSendParam*>(work_data);
	auto logger = prm->general.Logger;
	GrpId grp_id = prm->general.GroupId;
	auto& auth_facade = prm->general.AuthFacade;
	auto file_provider = prm->FileProvider;
	delete prm;

	auto acc_ref = AccCfg.FindAccount(grp_id);
	if (!acc_ref) return Error_Gen_ItemNotFound;
	AccountSettings acc(*acc_ref);

	if (proc_ctrl->StopFlag) return Error_Gen_Operation_Interrupted;
	// ** Loading mail messages list...
	auto mail_msg_list = file_provider(grp_id);
	if (mail_msg_list.empty()) return ResCode_NoContent;

	if (proc_ctrl->StopFlag) return Error_Gen_Operation_Interrupted;
	// ** Authenticating...
	std::string auth_data;
	int res_code = auth_facade.GetAuthData(auth_data, acc.Outgoing, proc_ctrl);
	if (res_code < 0) {
		logger->LogFmt(llError, Log_Scope " grp#%i Authentication failed, can't send mail: %i.", grp_id, res_code);
		return res_code;
	}

	if (proc_ctrl->StopFlag) return Error_Gen_Operation_Interrupted;
	// ** Sending mail...
	int file_count = 0;
	MailMsgTransmitter trns;
	MailMsgTransmitter::TransmissionHandle trn_handle;
	res_code = trns.BeginTransmition(grp_id, acc.Outgoing, auth_data.c_str(), acc.GetMailbox(), trn_handle);
	if (res_code >= 0) {
		for (auto& mail_msg_item : mail_msg_list) {
			MailMsgFile* mail_msg_file = mail_msg_item.get();
			int send_res = trns.SendMailMessage(trn_handle, *mail_msg_file);
			if (send_res >= 0) {
				mail_msg_file->SetMailAsSent();
				logger->LogFmt(llInfo, Log_Scope " grp#%i Mail message has been sent: ", grp_id);
				++file_count;
			} else {
				// TODO: provide some name or id to the log
				logger->LogFmt(llError, Log_Scope " grp#%i Mail message send failed %i.", grp_id, send_res);
			}
			if (proc_ctrl->StopFlag) { res_code = Error_Gen_Operation_Interrupted;  break; }
		}
	}
	trns.EndTransmission(trn_handle);

	// ** Finishing execution...
	if (res_code >= 0) {
		logger->LogFmt(llInfo, Log_Scope " grp#%i Mail messages sent: %i.", grp_id, file_count);
		res_code = file_count;
	} else {
		logger->LogFmt(llError, Log_Scope " grp#%i Couldn't send mail messages, code: %i.", grp_id, res_code);
	}

	return res_code;
}

std::string MailSyncMgr::GetGrpTaskProcId(GrpId grp_id, bool is_receiving)
{
	return std::to_string(grp_id) + (is_receiving ? "rcv" : "snd");
}
