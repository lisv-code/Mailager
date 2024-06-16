#pragma once
#include <string>
#include <LisCommon/FileSystem.h>

struct AppCfgData
{
	std::basic_string<FILE_PATH_CHAR> AppDataDir, TmpDataDir;
	long DefaultLogLevel = -1, MailMessageContentViewer = 0;
	std::string NetUserAgent;
};

class ApplicationConfiguration
{
	AppCfgData data;
public:
	void Load();
	const AppCfgData& Get() const { return data; }
};

extern ApplicationConfiguration AppCfg; // Application Configuration global singleton
