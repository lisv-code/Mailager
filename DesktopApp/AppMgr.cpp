#include "AppMgr.h"
#include <wx/stdpaths.h>
#include <wx/filefn.h>
#include <wx/fileconf.h>
#include <wx/image.h>
#include <wx/msgdlg.h>
#include "Resource.h"
#include <LisCommon/Logger.h>
#include "../CoreAppLib/AppDef.h"
#include "../CoreAppLib/AppResCodes.h"
#include "../CoreAppLib/AccountConfig.h"
#include "../CoreAppLib/OAuth2Config.h"
#include "AppCfg.h"
#include "Version.h"

#define AppLogFileNamePrefix "dsp"

#define _ADD_FILENAME(filename) + FILE_PATH_TEXT(FILE_PATH_SEPARATOR_STR filename)

#define Log_Scope "AppMgr"

#define Msg_ErrorInitLogger "Failed initializing logger"
#define Msg_ErrorCreateTmpDataDir "Failed creating temporary data directory"
#define Msg_ErrorCreateAppDataDir "Failed creating application data directory"

ApplicationManager AppMgr; // Application Manager global singleton

namespace ApplicationManager_Imp
{
	static int init_logger(long default_log_level = -1);
	static void log_app_version(LisLog::ILogger* logger);
}
using namespace ApplicationManager_Imp;

ApplicationManager::ApplicationManager() { }

ApplicationManager::~ApplicationManager()
{
#ifdef _WINDOWS
	if (!AppCfg.GetGeneral().TmpDataDir.empty()) {
		wxFileName::Rmdir(AppCfg.GetGeneral().TmpDataDir, wxPATH_RMDIR_FULL | wxPATH_RMDIR_RECURSIVE);
	}
#endif
}

int ApplicationManager::InitResources()
{
	wxInitAllImageHandlers(); // required for loading images other than BMP

	AppCfg.Load();

	int result = init_logger(AppCfg.GetGeneral().DefaultLogLevel);
	LisLog::ILogger* logger;
	if ((result < 0) || !(logger = LisLog::Logger::GetInstance())) {
		wxMessageBox(Msg_ErrorInitLogger, AppDef_Title, wxICON_ERROR);
		return result;
	}

	log_app_version(logger);
	logger->LogFmt(LisLog::LogLevel::llDebug, Log_Scope " General config: %s",
		wxString(AppCfg.GetCfgPath()).GetData().AsChar());

	if (!wxFileName::Mkdir(AppCfg.GetGeneral().TmpDataDir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)) {
		logger->LogFmt(LisLog::llError, Log_Scope Msg_ErrorCreateTmpDataDir,
			wxString(AppCfg.GetGeneral().TmpDataDir).GetData().AsChar());
		wxMessageBox(Msg_ErrorCreateTmpDataDir, AppDef_Title, wxICON_ERROR);
		return Error_Gen_Initialization;
	}
	if (!wxFileName::Mkdir(AppCfg.GetGeneral().UsrDataDir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)) {
		logger->LogFmt(LisLog::llError, Log_Scope Msg_ErrorCreateAppDataDir,
			wxString(AppCfg.GetGeneral().UsrDataDir).GetData().AsChar());
		wxMessageBox(Msg_ErrorCreateAppDataDir, AppDef_Title, wxICON_ERROR);
		return Error_Gen_Initialization;
	}

	// Config primary path is set to user-specific directory where modifications are saved by the user
	OAuth2Cfg.SetCfgPath((AppCfg.GetGeneral().UsrDataDir _ADD_FILENAME(AppDef_OAuth2CfgFileName)).c_str());
	// If no config specific to the user found, try to load from the Shared or Executable folders
	std::basic_string<FILE_PATH_CHAR> path1(AppCfg.GetGeneral().GlobalCfgDir _ADD_FILENAME(AppDef_OAuth2CfgFileName));
	std::basic_string<FILE_PATH_CHAR> path2(AppCfg.GetGeneral().AppExeDir _ADD_FILENAME(AppDef_OAuth2CfgFileName));
	const FILE_PATH_CHAR* alt_paths[] = { path1.c_str(), path2.c_str() };
	result = OAuth2Cfg.Load(alt_paths, sizeof(alt_paths) / sizeof(FILE_PATH_CHAR*));

	AccCfg.SetCfgPath((AppCfg.GetGeneral().UsrDataDir _ADD_FILENAME(AppDef_AccountsCfgFileName)).c_str());
	logger->LogFmt(LisLog::LogLevel::llDebug, Log_Scope " Account config: %s",
		wxString(AccCfg.GetCfgPath()).GetData().AsChar());
	result = AccCfg.Load();

	return result;
}

// ************************************ ApplicationManager_Imp *************************************

int ApplicationManager_Imp::init_logger(long default_log_level)
{
	if (default_log_level < 0) {
#ifdef NDEBUG
		default_log_level = LisLog::LogLevel::llInfo;
#else
		default_log_level = LisLog::LogLevel::llTrace;
#endif
	}

	LisLog::LogTargetBase* targets[] = {
		new LisLog::LogTargetTextFile(
			(AppCfg.GetGeneral().UsrDataDir
				+ FILE_PATH_TEXT(FILE_PATH_SEPARATOR_STR) + FILE_PATH_TEXT(AppDef_LogDirName)).c_str(),
			// TODO: log path should be /var/log/<app> on Linux
			FILE_PATH_TEXT(AppLogFileNamePrefix),
			(LisLog::LogLevel)default_log_level
		),
#ifndef NDEBUG
		new LisLog::LogTargetDebugOut(LisLog::LogLevel::llTrace)
#endif
	};

	int result = targets[0]->GetStatus();

	if (result >= 0)
		result = LisLog::Logger::InitSingleton(LisLog::LoggerSettings(),
			targets, sizeof(targets) / sizeof(LisLog::LogTargetBase*));
	else
		result += ErrResGrp_Logger;

	return result;
}

void ApplicationManager_Imp::log_app_version(LisLog::ILogger* logger)
{
	logger->LogFmt(LisLog::llInfo, Log_Scope " application version %s, %i-bit.",
		APP_VERSION, APP_BITNESS);
	// TODO: log application build additional info as platform/bitness
#ifndef NDEBUG
	logger->LogFmt(LisLog::llInfo, Log_Scope " running DEBUG build %s %s.", BUILD_DATE, BUILD_TIME);
#endif
}
