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
	etSyncFinished // MailMsgFileMgr_EventData_SyncFinish
};

typedef struct {
	const Connections::ConnectionInfo* Connection;
	std::string* PswdData;
	bool* NeedSave;
} MailMsgFileMgr_EventData_CredentialsRequest;
typedef std::shared_ptr<MailMsgFile> MailMsgFileMgr_EventData_NewMessage;
typedef struct { int GrpId; size_t ItemsCount; } MailMsgFileMgr_EventData_SyncFinish;

typedef EventDispatcherBase<MailMsgFileMgr, MailMsgFileMgr_EventType, void*> MailMsgFileMgr_EventDispatcher;

class MailMsgFileMgr : public MailMsgFileMgr_EventDispatcher
{
public:
	typedef int GrpId;
	typedef std::list<std::shared_ptr<MailMsgFile>> FilesContainer;
	typedef FilesContainer::const_iterator FilesIterator;

	enum MailMsgGrpStatus { mgsNone = 0, mgsProcessing = 1 };
private:
	typedef FilesContainer GrpDataItem;
	std::unordered_map<GrpId, GrpDataItem*> grpData;
	FilesContainer draftMessages;
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	LisThread::ThreadTaskMgr taskMgr;

	GrpDataItem* GetGrpData(GrpId grp_id, bool auto_create);
	void FreeGrpData(GrpDataItem* grp_data);
	int MailMsgFile_EventHandler(const MailMsgFile* mail_msg, const MailMsgFile::EventInfo& evt_info);

	struct MailSyncProcPrm
	{
		MailMsgFileMgr* Manager;
		AccountSettings Account;
	};
	static LisThread::TaskProcResult MailReceiveProc(
		LisThread::TaskProcCtrl* proc_ctrl, LisThread::TaskWorkData work_data);
	static LisThread::TaskProcResult MailSendProc(
		LisThread::TaskProcCtrl* proc_ctrl, LisThread::TaskWorkData work_data);

	static void AfterMailMsgFileAdded(MailMsgFileMgr* mgr, std::shared_ptr<MailMsgFile>* file);

	static int GetAuthData(std::string& auth_data, const Connections::ConnectionInfo& connection,
		LisThread::TaskProcCtrl* proc_ctrl, const MailMsgFileMgr* mgr);
	static bool AuthEvtProc_UserPswd(ConnectionAuth::EventParams& event_params,
		const Connections::ConnectionInfo& connection, const MailMsgFileMgr* mgr);
	static bool AuthEvtProc_OAuth2(ConnectionAuth::EventParams& event_params,
		LisThread::TaskProcCtrl* proc_ctrl);
public:
	~MailMsgFileMgr();
	MailMsgGrpStatus GetProcStatus(GrpId grp_id);
	bool GetIter(GrpId grp_id, FilesIterator& begin, FilesIterator& end);
	bool LoadList(GrpId grp_id, const char* acc_dir);
	bool StopProcessing(GrpId grp_id);
	bool RemoveGroup(GrpId grp_id);
	bool StartMailSync(GrpId grp_id, const AccountSettings& account);
	std::shared_ptr<MailMsgFile>& CreateMailMsg(GrpId grp_id);
};
