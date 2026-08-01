#include "MailMsgFileMgr.h"
#include "../CoreAppLib/AppResCodes.h"
#include "AppCfg.h"

namespace MailMsgFileMgr_Imp
{
	static bool is_mail_msg_to_send(const MailMsgFile& mail_msg);
	static bool is_mail_msg_status_needs_monitoring(const MailMsgFile& mail_msg);
}
using namespace MailMsgFileMgr_Imp;

MailMsgFileMgr::MailMsgFileMgr()
{
	auto handler = std::bind(&MailMsgFileMgr::MailMsgRegistry_EventHandler,
		this, std::placeholders::_1, std::placeholders::_2);
	MsgRegistry.EventSubscribe(MailMsgRegistry_EventType::NewMsgFile, handler);

	auto& gen_cfg = AppCfg.GetGeneral();
	SyncMgr.AuthFacade.AuthDataBaseDir = gen_cfg.UsrDataDir;
	SyncMgr.TempDataDir = gen_cfg.TmpDataDir;
	SyncMgr.UserDataDir = gen_cfg.UsrDataDir;
}

MailSyncProcStatus MailMsgFileMgr::GetSyncStatus(GrpId grp_id)
{
	return SyncMgr.GetProcStatus(grp_id);
}

int MailMsgFileMgr::StartMailSync(GrpId grp_id, bool receive, bool send)
{
	int result = MsgRegistry.InitGroup(grp_id); // Ensure the registry contains the group
	if (result _Is_Ok_ResCode) {
		if (receive) {
			auto file_consumer = std::bind(&MailMsgFileMgr::SyncRecvFileConsumer, this, std::placeholders::_1);
			result = SyncMgr.StartMailRecv(grp_id, file_consumer);
		}
		if (send) {
			auto file_provider = std::bind(&MailMsgFileMgr::SyncSendFileProvider, this, std::placeholders::_1);
			result = SyncMgr.StartMailSend(grp_id, file_provider);
		}
	}
	return result;
}

bool MailMsgFileMgr::StopMailSync(GrpId grp_id, bool receiving, bool sending)
{
	bool result = false;
	if (sending) result = SyncMgr.StopMailSend(grp_id);
	if (receiving) result = result || SyncMgr.StopMailRecv(grp_id);
	return result;
}

bool MailMsgFileMgr::RemoveGroup(GrpId grp_id)
{
	auto& mail_sync_mgr = SyncMgr;
	mail_sync_mgr.StopMailSend(grp_id);
	mail_sync_mgr.StopMailRecv(grp_id);
	return MsgRegistry.RemoveGroup(grp_id);
}

int MailMsgFileMgr::SyncRecvFileConsumer(MailMsgFile& msg_file)
{
	return MsgRegistry.EmplaceFile(msg_file);
}

std::vector<std::shared_ptr<MailMsgFile>> MailMsgFileMgr::SyncSendFileProvider(GrpId grp_id)
{
	std::vector<std::shared_ptr<MailMsgFile>> mail_msg_list;
	if (auto msg_reg_data = MsgRegistry.GetFileDataView(grp_id)) {
		for (auto it = msg_reg_data.Begin; it != msg_reg_data.End; ++it) {
			if (is_mail_msg_to_send(*it->get())) mail_msg_list.push_back(*it);
		}
	} // else Error_Gen_ItemNotFound
	return mail_msg_list;
}

int MailMsgFileMgr::MailMsgRegistry_EventHandler(const MailMsgRegistry* registry, const MailMsgRegistry::EventInfo& evt_info)
{
	int result = ResCode_Ok;
	if (MailMsgRegistry_EventType::NewMsgFile == evt_info.type) {
		// New MsgFile has been added
		const auto& evt_data = evt_info.data;
		AfterMailMsgFileAdded(evt_data, this);
	}
	return result;
}

int MailMsgFileMgr::MailMsgFile_EventHandler(const MailMsgFile* mail_msg, const MailMsgFile::EventInfo& evt_info)
{
	int result = ResCode_Ok;
	if (MailMsgFile_EventType::StatusChanged == evt_info.type) {
		// File status changed - Check if message is ready to be sent and enqueue it
		if (is_mail_msg_to_send(*mail_msg)) {
			auto file_provider = std::bind(&MailMsgFileMgr::SyncSendFileProvider, this, std::placeholders::_1);
			SyncMgr.StartMailSend(mail_msg->GetGrpId(), file_provider);
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

void MailMsgFileMgr::AfterMailMsgFileAdded(const std::shared_ptr<MailMsgFile>& file, MailMsgFileMgr* mgr)
{
	if (is_mail_msg_status_needs_monitoring(*file)) {
		// Subscribe to the MsgFile events
		auto handler = GetMailMsgFileEvtHandler(mgr);
		file->EventSubscribe(MailMsgFile_EventType::StatusChanged, handler);
	}
}

bool MailMsgFileMgr_Imp::is_mail_msg_to_send(const MailMsgFile& mail_msg)
{
	return mail_msg.CheckStatusFlags(
		MailMsgStatus::mmsIsOutgoing,
		MailMsgStatus::mmsIsDraft | MailMsgStatus::mmsIsSent | MailMsgStatus::mmsIsDeleted);
}

bool MailMsgFileMgr_Imp::is_mail_msg_status_needs_monitoring(const MailMsgFile& mail_msg)
{
	return mail_msg.CheckStatusFlags(
		MailMsgStatus::mmsIsDraft | MailMsgStatus::mmsIsOutgoing,
		MailMsgStatus::mmsIsSent | MailMsgStatus::mmsIsDeleted);
}
