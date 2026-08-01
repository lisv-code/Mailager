#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <LisCommon/EventDispBase.h>
#include <LisCommon/Logger.h>
#include <LisCommon/ThreadTaskMgr.h>
#include "ConnAuthFacade.h"
#include "MailMsgFile.h"

enum class MailSyncProcStatus { spsNone = 0, spsReceiving = 1, spsSending = 2 };

inline int operator&(MailSyncProcStatus a, MailSyncProcStatus b)
{
	return static_cast<int>(a) & static_cast<int>(b);
}
inline MailSyncProcStatus operator|(MailSyncProcStatus a, MailSyncProcStatus b)
{
	return static_cast<MailSyncProcStatus>(static_cast<int>(a) | static_cast<int>(b));
}

class MailSyncMgr; // forward declaration

enum class MailSyncMgr_EventType { RecvFinished, SendFinished };
struct MailSyncMgr_EventData {
	int GrpId;
	LisThread::TaskProcResult ResCode;
	MailSyncMgr_EventData(int grp_id, LisThread::TaskProcResult res_code) : GrpId(grp_id), ResCode(res_code) { }
};
typedef EventDispatcherBase<MailSyncMgr, MailSyncMgr_EventType, const MailSyncMgr_EventData&> MailSyncMgr_EvtDisp;

class MailSyncMgr : public MailSyncMgr_EvtDisp
{
public:
	typedef int GrpId;
	typedef std::function<int(MailMsgFile& msg_file)> FileConsumer;
	typedef std::function<std::vector<std::shared_ptr<MailMsgFile>>(GrpId grp_id)> FileProvider;
	ConnAuthFacade AuthFacade;
	std::basic_string<FILE_PATH_CHAR> TempDataDir, UserDataDir;

	MailSyncProcStatus GetProcStatus(GrpId grp_id);

	int StartMailRecv(GrpId grp_id, FileConsumer file_consumer);
	int StartMailSend(GrpId grp_id, FileProvider file_provider);
	bool StopMailRecv(GrpId grp_id);
	bool StopMailSend(GrpId grp_id);

private:
	LisThread::ThreadTaskMgr taskMgr;
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();

	static std::string GetGrpTaskProcId(GrpId grp_id, bool is_receiving);

	struct MailSyncProcGenParam {
		LisLog::ILogger* Logger;
		GrpId GroupId;
		ConnAuthFacade& AuthFacade;
	};
	struct MailSyncProcRecvParam {
		MailSyncProcGenParam general;
		std::basic_string<FILE_PATH_CHAR> TmpDataDir, UsrDataDir;
		FileConsumer FileConsumer;
	};
	struct MailSyncProcSendParam {
		MailSyncProcGenParam general;
		FileProvider FileProvider;
	};
	static LisThread::TaskProcResult MailRecvProc(
		LisThread::TaskProcCtrl* proc_ctrl, LisThread::TaskWorkData work_data);
	static LisThread::TaskProcResult MailSendProc(
		LisThread::TaskProcCtrl* proc_ctrl, LisThread::TaskWorkData work_data);
};
