#pragma once
#include "ContentViewer.h"
#include <wx/webview.h>
#include <LisCommon/Logger.h>

class WebContentViewer : public ContentViewer
{
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	wxWebView* viewCtrl;
	bool hasExternalImages;

	void OpenUrl(const wxString& url);

	void OnWebViewLoaded(wxWebViewEvent& event);
	void OnWebViewError(wxWebViewEvent& event);
	void OnWebViewNavigating(wxWebViewEvent& event);
	void OnWebViewNewWindow(wxWebViewEvent& event);
public:
	WebContentViewer();
	virtual ~WebContentViewer() override;
	virtual wxWindow* InitView(wxWindow* parent, wxWindow* container) override;
	virtual bool SetContentDataProvider(ContentDataProvider proc) override;
	virtual bool SetContent(const wxChar* content) override;
	virtual bool ReloadContent() override;
	virtual bool GetHasExternalImages() override;
};
