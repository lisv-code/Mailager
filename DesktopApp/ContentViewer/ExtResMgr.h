#pragma once
#include <sstream>
#include <unordered_map>
#include <LisCommon/Logger.h>
#include <LisCommon/ThreadTaskMgr.h>

class ExtResMgr
{
	typedef std::string ExtResId;
	struct ExtResItem {
		std::stringstream Data;
		bool IsDataAvailable() { return Data.rdbuf()->in_avail(); }
	};
	std::unordered_map<std::string, ExtResItem*> extResItems;
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	LisThread::ThreadTaskMgr taskMgr;

	ExtResMgr::ExtResItem* GetResItem(const ExtResId& res_id, bool auto_create);

	struct ResourceDownloadProcPrm
	{
		ExtResMgr* Manager;
		std::string Url;
	};
	static LisThread::TaskProcResult ResourceDownloadProc(
		LisThread::TaskProcCtrl* proc_ctrl, LisThread::TaskWorkData work_data);
public:
	ExtResMgr();
	~ExtResMgr();
	static ExtResMgr* GetInstance();
	static std::string SanitizeUrl(const char* url);
	bool StartDownload(const char* url);
	bool GetResourceData(const char* url, std::istream** data, bool peek_cache_only);
};
