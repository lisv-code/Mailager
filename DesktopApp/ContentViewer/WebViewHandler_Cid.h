#pragma once
#include <wx/webview.h>
#include "ContentViewer.h"
#include "UrlSchemeHandler_Cid.h"

class WebViewHandler_Cid : public wxWebViewHandler
{
	UrlSchemeHandler_Cid schemeHandler;
public:
	WebViewHandler_Cid(const wxString& scheme, ContentViewer::ContentDataProvider data_provider);
	virtual wxFSFile* GetFile(const wxString& uri) override;
};
