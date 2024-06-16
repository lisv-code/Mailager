#pragma once
#include "ContentViewer.h"
#include <wx/html/htmlwin.h>
#include <LisCommon/Logger.h>

class HtmlContentViewer : public ContentViewer
{
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	wxHtmlWindow* viewCtrl;
	wxString contentCache;
	std::string schemePrefix;
	wxFileSystemHandler *cidHandler;

public:
	HtmlContentViewer();
	virtual ~HtmlContentViewer() override;
	virtual wxWindow* InitView(wxWindow* parent, wxWindow* container) override;
	virtual bool SetContentDataProvider(ContentDataProvider proc) override;
	virtual bool GetExtResourcesDownload() override;
	virtual bool SetExtResourcesDownload(bool is_allowed) override;
	virtual bool SetContent(const wxChar* content) override;
	virtual bool ReloadContent() override;
	virtual bool GetHasExternalImages() override;
};
