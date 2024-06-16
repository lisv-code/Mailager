#include "WebViewHandler_Cid.h"

WebViewHandler_Cid::WebViewHandler_Cid(const wxString& scheme, ContentViewer::ContentDataProvider data_provider)
	: wxWebViewHandler(scheme), schemeHandler(data_provider)
{ }

wxFSFile* WebViewHandler_Cid::GetFile(const wxString& uri)
{
	static wxFileSystem fs;
	return schemeHandler.OpenFile(fs, uri);
}
