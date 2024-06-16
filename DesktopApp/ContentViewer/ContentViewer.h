#pragma once
#include <istream>
#include <wx/window.h>

class ContentViewer
{
public:
	struct ContentData { std::string type; std::istream* data; };
	typedef std::function<ContentData(const wxChar* id)> ContentDataProvider;

	virtual ~ContentViewer() { };
	virtual wxWindow* InitView(wxWindow* parent, wxWindow* container) = 0;
	virtual bool SetContentDataProvider(ContentDataProvider proc) { return false; }
	virtual bool GetExtResourcesDownload() { return false; }
	virtual bool SetExtResourcesDownload(bool is_allowed) { return false; }
	virtual bool SetContent(const wxChar* content) = 0;
	virtual bool ReloadContent() = 0;
	virtual bool GetHasExternalImages() { return false; }
};
