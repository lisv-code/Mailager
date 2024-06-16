#pragma once
#include "LogViewUI.h"
#include <LisCommon/Logger.h>

namespace LogView_Def
{
	extern const wxChar* WndTitle;
}

class LogView : public LogViewUI
{
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	LisLog::ILogger::TargetHandle logTarget;

	void LogTargetFunc(const char* txt);
	void LogWriteEventHandler(wxCommandEvent& event);
public:
	LogView(wxWindow* parent);
	~LogView();
};
