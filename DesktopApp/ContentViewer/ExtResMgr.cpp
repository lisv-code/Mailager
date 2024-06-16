#include "ExtResMgr.h"
#include "../../CoreNetLib/NetClient.h"
#include "../AppCfg.h"

#define Log_Scope "ExtResMgr"

namespace ExtResMgr_Imp
{
	const int ThreadRetCode_Ok = 0;
	const int ThreadRetCode_Failure = -1023;
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

	LisThread::TaskProcResult res_code = ThreadRetCode_Failure;
	std::stringstream stm(std::ios::in | std::ios::out | std::ios::binary);
	NetClient net_client;
	net_client.SetDefaultTimeout(TransferTimeoutMs);
	net_client.SetDefaultUserAgent(AppCfg.Get().NetUserAgent.c_str());
	int result = net_client.Exec(stm, url.c_str(), nullptr);
	if (0 == result && !stm.fail()) {
		auto res_item = mgr->GetResItem(url, false);
		if (res_item) {
			res_item->Data.swap(stm);
			res_code = ThreadRetCode_Ok;
		}
	}
	return res_code;
}
