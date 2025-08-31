#pragma once
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <LisCommon/Logger.h>
#include <LisCommon/ThreadTaskMgr.h>
#include "../CoreAppLib/AccountSettings.h"
#include "../CoreAppLib/MailMsgFile.h"
#include "../CoreAppLib/ConnectionAuth.h"

class MailMsgFileMgr; // forward declaration

enum MailMsgFileMgr_EventType
{
	etCredentialsRequest, // MailMsgFileMgr_EventData_CredentialsRequest
	etNewMessageAdded, // MailMsgFileMgr_EventData_NewMessage
	etRecvFinished, // MailMsgFileMgr_EventData_SyncFinish
	etSendFinished // MailMsgFileMgr_EventData_SyncFinish
};

struct MailMsgFileMgr_EventData_CredentialsRequest {
	const Connections::ConnectionInfo* Connection;
	std::string* PswdData;
	bool* NeedSave;
};
typedef std::shared_ptr<MailMsgFile> MailMsgFileMgr_EventData_NewMessage;
struct MailMsgFileMgr_EventData_SyncFinish {
	int GrpId;
	LisThread::TaskProcResult ResCode;
	MailMsgFileMgr_EventData_SyncFinish(int grp_id, LisThread::TaskProcResult res_code)
		: GrpId(grp_id), ResCode(res_code) { }
};

typedef EventDispatcherBase<MailMsgFileMgr, MailMsgFileMgr_EventType, void*> MailMsgFileMgr_EventDispatcher;

class MailMsgFileMgr : public MailMsgFileMgr_EventDispatcher
{
public:
	typedef int GrpId;
	typedef std::list<std::shared_ptr<MailMsgFile>> FilesContainer;
	typedef FilesContainer::const_iterator FilesIterator;

	enum GrpProcStatus { gpsNone = 0, gpsProcReceiving = 1, gpsProcSending = 2 };
private:
	struct GrpDataItem {
		AccountSettings MailAcc;
		FilesContainer MsgFiles;
	};
	std::unordered_map<GrpId, GrpDataItem*> mailGroups;
	FilesContainer draftMessages;
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	LisThread::ThreadTaskMgr taskMgr;

	GrpDataItem* GetGrpData(GrpId grp_id);
	void FreeGrpData(GrpDataItem* grp_data);
	int MailMsgFile_EventHandler(const MailMsgFile* mail_msg, const MailMsgFile::EventInfo& evt_info);

	static MailMsgFile::EventHandler GetMailMsgFileEvtHandler(MailMsgFileMgr* mgr);
	static void AfterMailMsgFileAdded(std::shared_ptr<MailMsgFile>* file, MailMsgFileMgr* mgr, bool raise_event);

	static std::string GetGrpTaskProcId(GrpId grp_id, bool is_receiving);

	struct MailSyncProcPrm
	{
		MailMsgFileMgr* Manager;
		GrpId GroupId;
	};
	static LisThread::TaskProcResult MailRecvProc(
		LisThread::TaskProcCtrl* proc_ctrl, LisThread::TaskWorkData work_data);
	static LisThread::TaskProcResult MailSendProc(
		LisThread::TaskProcCtrl* proc_ctrl, LisThread::TaskWorkData work_data);

	static int GetAuthData(std::string& auth_data, const Connections::ConnectionInfo& connection,
		LisThread::TaskProcCtrl* proc_ctrl, const MailMsgFileMgr* mgr);
	static bool AuthEvtProc_UserPswd(ConnectionAuth::EventData_PswdRequest* pswd_data,
		const Connections::ConnectionInfo& connection, const MailMsgFileMgr* mgr);
	static bool AuthEvtProc_StopFunc(ConnectionAuth::EventData_StopFunction* stop_func,
		LisThread::TaskProcCtrl* proc_ctrl);
public:
	~MailMsgFileMgr();
	int InitGroup(GrpId grp_id, const AccountSettings& account);
	GrpProcStatus GetProcStatus(GrpId grp_id);
	bool GetIter(GrpId grp_id, FilesIterator& begin, FilesIterator& end);
	int LoadList(GrpId grp_id);
	bool RemoveGroup(GrpId grp_id);
	int StartMailRecv(GrpId grp_id);
	int StartMailSend(GrpId grp_id);
	bool StopMailRecv(GrpId grp_id);
	bool StopMailSend(GrpId grp_id);
	std::shared_ptr<MailMsgFile>& CreateMailMsg(GrpId grp_id);
};
