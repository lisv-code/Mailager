#pragma once
#include <string>
#include <LisCommon/FileSystem.h>

class ApplicationConfiguration
{
public:
	struct GeneralSettings {
		std::basic_string<FILE_PATH_CHAR> AppExeDir;
		std::basic_string<FILE_PATH_CHAR> GlobalCfgDir, UsrDataDir, TmpDataDir, ShrDataDir;
		long DefaultLogLevel = -1, MailMessageContentViewer = 0;
	};

	void Load();
	const FILE_PATH_CHAR* GetCfgPath() const { return cfgPath.c_str(); }
	const GeneralSettings& GetGeneral() const { return cfgGenData; }
private:
	std::basic_string<FILE_PATH_CHAR> cfgPath;
	GeneralSettings cfgGenData;
};

extern ApplicationConfiguration AppCfg; // Application Configuration global singleton
