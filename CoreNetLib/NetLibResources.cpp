#include "NetLibResources.h"
#include <mutex>
#include <curl/curl.h>
#include <LisCommon/Logger.h>

using namespace NetLibResources;
namespace NetLibResources_Imp
{
#define Log_Scope "NetRsc"

	static int NetLibUsageGlobalCount = 0;
	static std::mutex NetLibUsageGlobalMutex;
}
using namespace NetLibResources_Imp;
using namespace LisLog;

void NetLibResources::global_init()
{
	std::lock_guard<std::mutex> lock(NetLibUsageGlobalMutex);
	if (0 == NetLibUsageGlobalCount) {
		CURLcode res_code = curl_global_init(CURL_GLOBAL_DEFAULT);
		LisLog::ILogger* logger = LisLog::Logger::GetInstance();
		if (logger)
			logger->LogFmt(llDebug, Log_Scope " global Init result: %i.", static_cast<int>(res_code));
	}
	++NetLibUsageGlobalCount;
}

void NetLibResources::global_free()
{
	std::lock_guard<std::mutex> lock(NetLibUsageGlobalMutex);
	--NetLibUsageGlobalCount;
	if (NetLibUsageGlobalCount <= 0) {
		curl_global_cleanup();
		NetLibUsageGlobalCount = 0;
		LisLog::ILogger* logger = LisLog::Logger::GetInstance();
		if (logger)
			logger->LogTxt(llDebug, Log_Scope " global Cleanup done.");
	}
}
