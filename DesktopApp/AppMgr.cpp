#include "AppMgr.h"
#include <wx/stdpaths.h>
#include <wx/filefn.h>
#include <wx/fileconf.h>
#include <wx/image.h>
#include <wx/msgdlg.h>
#include "Resource.h"
#include <LisCommon/Logger.h>
#include "../CoreAppLib/AppDef.h"
#include "../CoreAppLib/AccountCfg.h"
#include "AppCfg.h"

#define AppLogFileDirectory "logs"
#define AppLogFileNamePrefix "dsp"

#define Result_ErrorCreateTmpDataDir -1
#define Result_ErrorCreateAppDataDir -2

#define Result_ShiftErrorLogStatus -10

#define Msg_ErrorCreateTmpDataDir "Failed creating temporary data directory"
#define Msg_ErrorCreateAppDataDir "Failed creating application data directory"

ApplicationManager AppMgr; // Application Manager global singleton

ApplicationManager::ApplicationManager() { }

ApplicationManager::~ApplicationManager()
{
#ifdef _WINDOWS
	if (!AppCfg.Get().TmpDataDir.empty()) {
		wxFileName::Rmdir(AppCfg.Get().TmpDataDir, wxPATH_RMDIR_FULL | wxPATH_RMDIR_RECURSIVE);
	}
#endif
}

int ApplicationManager::InitResources()
{
	wxInitAllImageHandlers(); // required for loading images other than BMP

	AppCfg.Load();

	if (!wxFileName::Mkdir(AppCfg.Get().TmpDataDir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)) {
		wxMessageBox(Msg_ErrorCreateTmpDataDir, AppDef_Title, wxICON_ERROR);
		return Result_ErrorCreateTmpDataDir;
	}
	if (!wxFileName::Mkdir(AppCfg.Get().AppDataDir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)) {
		wxMessageBox(Msg_ErrorCreateAppDataDir, AppDef_Title, wxICON_ERROR);
		return Result_ErrorCreateAppDataDir;
	}

	int result = InitLogger(AppCfg.Get().DefaultLogLevel);

	if (result >= 0) {
		AccCfg.SetFilePath(
			(AppCfg.Get().AppDataDir + FILE_PATH_TEXT(FILE_PATH_SEPARATOR_STR "accounts.ini")).c_str());
		result = AccCfg.Load();
	}

	return result;
}

int ApplicationManager::InitLogger(long default_log_level)
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
			(AppCfg.Get().AppDataDir
				+ FILE_PATH_TEXT(FILE_PATH_SEPARATOR_STR) + FILE_PATH_TEXT(AppLogFileDirectory)).c_str(),
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
		result += Result_ShiftErrorLogStatus;

	return result;
}
