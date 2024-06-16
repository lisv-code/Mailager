#pragma once
#include <wx/webview.h>
#include <LisCommon/Logger.h>

class WebViewHandler_Null : public wxWebViewHandler
{
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	bool* queryFlag;
public:
	WebViewHandler_Null(const wxString& scheme, bool* is_queried_flag);
	virtual wxFSFile* GetFile(const wxString& uri) override;
};
