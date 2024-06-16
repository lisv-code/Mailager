#include "WebViewHandler_Null.h"

namespace WebViewHandler_Null_Imp
{
#define LogScope "WebView_Null"
}
using namespace WebViewHandler_Null_Imp;

WebViewHandler_Null::WebViewHandler_Null(const wxString& scheme, bool* is_queried_flag)
	: wxWebViewHandler(scheme)
{
	queryFlag = is_queried_flag;
}

wxFSFile* WebViewHandler_Null::GetFile(const wxString& uri)
{
	logger->LogFmt(LisLog::llTrace, LogScope " GetFile %s", (char*)uri.char_str());
	if (queryFlag) {
		*queryFlag = true;
	}
	return NULL;
}
