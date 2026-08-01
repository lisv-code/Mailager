#pragma once
#include <memory>
#include <vector>
#include "../CoreAppLib/MailMsgFile.h"
#include "../CoreAppLib/MailSyncMgr.h"
#include "MailMsgRegistry.h"

class MailMsgFileMgr
{
public:
	typedef int GrpId;
	MailMsgRegistry MsgRegistry;
	MailSyncMgr SyncMgr;

	MailMsgFileMgr();

	MailSyncProcStatus GetSyncStatus(GrpId grp_id);
	int StartMailSync(GrpId grp_id, bool receive, bool send);
	bool StopMailSync(GrpId grp_id, bool receiving, bool sending);

	bool RemoveGroup(GrpId grp_id);

private:
	int SyncRecvFileConsumer(MailMsgFile& msg_file);
	std::vector<std::shared_ptr<MailMsgFile>> SyncSendFileProvider(GrpId grp_id);

	int MailMsgRegistry_EventHandler(const MailMsgRegistry* registry, const MailMsgRegistry::EventInfo& evt_info);
	int MailMsgFile_EventHandler(const MailMsgFile* mail_msg, const MailMsgFile::EventInfo& evt_info);

	static MailMsgFile::EventHandler GetMailMsgFileEvtHandler(MailMsgFileMgr* mgr);
	static void AfterMailMsgFileAdded(const std::shared_ptr<MailMsgFile>& file, MailMsgFileMgr* mgr);
};
