#include "ExtResMgr.h"
#include <cstring>
#include "../../CoreAppLib/AppResCodes.h"
#include "../../CoreNetLib/NetClient.h"
#include "../AppCfg.h"

#define Log_Scope "ExtResMgr"

namespace ExtResMgr_Imp
{
	const char *SpaceCharsStr = " \t\n\r\f\v";
	const size_t SpaceCharsLen = std::strlen(SpaceCharsStr);

	const int TaskWaitFinishMs = 20000; // 20 seconds
	const int DownloadAttemptDelayMs = 6000;

	const int TransferTimeoutMs = 16000; // 16 seconds
}
using namespace ExtResMgr_Imp;
using namespace LisLog;

static ExtResMgr* ExtResMgr_Singleton = nullptr; // Global singleton

ExtResMgr::ExtResMgr() : taskMgr(true) { }

ExtResMgr::~ExtResMgr()
{
	for (auto& item : extResItems) { delete item.second; }
}

ExtResMgr* ExtResMgr::GetInstance()
{
	if (!ExtResMgr_Singleton) ExtResMgr_Singleton = new ExtResMgr;
	return ExtResMgr_Singleton;
}

std::string ExtResMgr::SanitizeUrl(const char* url)
{
	if (!url) return std::string();

	std::string str;
	str.reserve(std::strlen(url));
	size_t src_pos = 0;
	bool is_start_check = true, is_prev_dot = false;
	unsigned char chr;
	while (chr = url[src_pos++]) {
		if (is_start_check) { // Skip non-ASCII-alpha characters
			if ((chr >= 'A' && chr <= 'Z') || (chr >= 'a' && chr <= 'z'))
				is_start_check = false;
			else continue;
		} else { // Skip whitespaces and collapse consecutive dots (".." or ". .") into a single "."
			if (std::memchr(SpaceCharsStr, chr, SpaceCharsLen))
				continue;
			if ('.' == chr) {
				if (is_prev_dot) continue;
				is_prev_dot = true;
			} else is_prev_dot = false;
		}
		str.push_back(chr);
	}

	return str;
}

bool ExtResMgr::StartDownload(const char* url)
{
	auto res_item = GetResItem(url, true); // If not exists yet, is created now
	if (!res_item) return false; // Unlikely to happen, but potential failure if memory not enough
	if (res_item->IsDataAvailable()) return true; // Looks like it's already downloaded

	auto task_status = taskMgr.GetTaskStatus(url);
	bool is_start_needed = LisThread::TaskProcStatus::tpsNone == task_status;
	if (!is_start_needed && (LisThread::TaskProcStatus::tpsFinished == task_status)) {
		auto fin_time = taskMgr.GetTaskTime(url, LisThread::TimeValueType::tvtFinish);
		if (LisThread::TimeValue_Empty == fin_time) {
			is_start_needed = !res_item->IsDataAvailable(); // Possible became available since previous check
		} else {
			auto time_delta = std::chrono::system_clock::now() - fin_time;
			is_start_needed = time_delta >= std::chrono::milliseconds(DownloadAttemptDelayMs);
		}
	}
	if (is_start_needed) {
		auto proc = std::bind(&ExtResMgr::ResourceDownloadProc, std::placeholders::_1, std::placeholders::_2);
		auto data = new ResourceDownloadProcPrm{ this, url };
		auto result = taskMgr.StartTask(url, proc, data);
		if (result)
			logger->LogFmt(llInfo, Log_Scope " download: %s", url);
		else
			logger->LogFmt(llError, Log_Scope " download start failed: %s", url);
		return true;
	} else if (LisThread::TaskProcStatus::tpsProcessing == task_status || res_item->IsDataAvailable()) {
		return true;
	}
	return false;
}

bool ExtResMgr::GetResourceData(const char* url, std::istream** data, bool peek_cache_only)
{
	auto res_item = GetResItem(url, true);
	if (!res_item->IsDataAvailable() && !peek_cache_only) {
		if (StartDownload(url)) {
			taskMgr.WaitTask(url, TaskWaitFinishMs); // TODO: ? if wait failed, stop the task
		}
	}
	if (res_item->IsDataAvailable()) {
		if (data) *data = static_cast<std::istream*>(&res_item->Data);
		return true;
	}
	else
		return false;
}

ExtResMgr::ExtResItem* ExtResMgr::GetResItem(const ExtResId& res_id, bool auto_create)
{
	ExtResItem* result = nullptr;
	const auto it = extResItems.find(res_id);
	if (it == extResItems.end()) {
		if (auto_create) {
			auto acc_data = new ExtResItem{};
			auto item = extResItems.insert(std::make_pair(res_id, acc_data));
			result = (*item.first).second;
		}
	} else {
		result = (*it).second;
	}
	return result;
}

LisThread::TaskProcResult ExtResMgr::ResourceDownloadProc(
	LisThread::TaskProcCtrl* proc_ctrl, LisThread::TaskWorkData work_data)
{
	auto prm = static_cast<ResourceDownloadProcPrm*>(work_data);
	auto mgr = prm->Manager;
	std::string url(prm->Url);
	delete prm;

	std::stringstream stm(std::ios::in | std::ios::out | std::ios::binary);
	NetClient net_client;
	net_client.SetDefaultTimeout(TransferTimeoutMs);
	net_client.SetDefaultUserAgent(AppCfg.Get().NetUserAgent.c_str());
	int result = ResCode_OfNetLib(net_client.Exec(stm, url.c_str(), nullptr));
	if ((result _Is_Ok_ResCode) && stm.fail())
		result = Error_File_DataOperation;
	if (result _Is_Ok_ResCode) {
		auto res_item = mgr->GetResItem(url, false);
		if (res_item) res_item->Data.swap(stm);
		else result = Error_Gen_ItemNotFound;
	}
	return static_cast<LisThread::TaskProcResult>(result);
}
