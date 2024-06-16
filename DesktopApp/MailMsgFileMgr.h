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

namespace MailMsgFileMgr_Def
{
	enum MailMsgGrpStatus { mgsNone = 0, mgsProcessing = 1 };
}

class MailMsgFileMgr
{
public:
	typedef int GrpId;
	typedef std::list<std::shared_ptr<MailMsgFile>> FilesContainer;
	typedef FilesContainer::const_iterator FilesIterator;

	enum ProcEventType { petCredentialsRequest, petNewMessageAdded, petSyncFinished };
	struct ProcEventData {
		ProcEventType Type;
		union {
			struct {
				const Connections::ConnectionInfo* Connection;
				std::string* PswdData;
				bool* NeedSave;
			} CredReq;
			std::shared_ptr<MailMsgFile>* MailMsg;
			size_t ItemsCount;
		};
	};
	typedef std::function<bool(GrpId grp_id, ProcEventData& data)> ProcEventHandler;
private:
	typedef FilesContainer GrpDataItem;
	std::unordered_map<GrpId, GrpDataItem*> grpData;
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	LisThread::ThreadTaskMgr taskMgr;

	GrpDataItem* GetGrpData(GrpId grp_id, bool auto_create);
	void FreeGrpData(GrpDataItem* grp_data);
	int MailMsgFile_EventHandler(const MailMsgFile* mail_msg, const MailMsgFile::EventInfo& evt_info);

	struct MailSyncProcPrm
	{
		MailMsgFileMgr* Manager;
		GrpId GroupId;
		AccountSettings Account;
		ProcEventHandler Callback;
	};
	static LisThread::TaskProcResult MailRetrieveProc(
		LisThread::TaskProcCtrl* proc_ctrl, LisThread::TaskWorkData work_data);

	static void AfterMailMsgFileAdded(MailMsgFileMgr* mgr, int grp_id, std::shared_ptr<MailMsgFile>* file,
		const ProcEventHandler& callback);

	static int GetAuthData(std::string& auth_data,
		GrpId grp_id, const Connections::ConnectionInfo& connection,
		LisThread::TaskProcCtrl* proc_ctrl, const ProcEventHandler& callback);
	static bool AuthEvtProc_UserPswd(ConnectionAuth::EventParams& event_params,
		GrpId grp_id, const Connections::ConnectionInfo& connection, const ProcEventHandler& callback);
	static bool AuthEvtProc_OAuth2(ConnectionAuth::EventParams& event_params,
		LisThread::TaskProcCtrl* proc_ctrl);
public:
	~MailMsgFileMgr();
	MailMsgFileMgr_Def::MailMsgGrpStatus GetProcStatus(GrpId grp_id);
	bool GetIter(GrpId grp_id, FilesIterator& begin, FilesIterator& end);
	bool LoadList(GrpId grp_id, const char* acc_dir);
	bool StopProcessing(GrpId grp_id);
	bool RemoveGroup(GrpId grp_id);
	bool StartSyncMail(GrpId grp_id, const AccountSettings& account, ProcEventHandler callback);
};
