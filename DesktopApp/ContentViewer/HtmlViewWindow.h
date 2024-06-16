#pragma once
#include <deque>
#include <wx/html/htmlwin.h>
#include <LisCommon/Logger.h>

struct HtmlViewStatus
{
	std::deque<wxString> ExternalImages;
	void Clear() { ExternalImages.clear(); }
};

class HtmlViewWindow : public wxHtmlWindow
{
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	wxString schemePrefix;
	mutable HtmlViewStatus status;
	bool isExtDownload;
public:
	HtmlViewWindow(wxWindow* parent, wxWindowID id, const char* scheme_prefix);
	virtual void OnLinkClicked(const wxHtmlLinkInfo& link) override;
	virtual wxHtmlOpeningStatus OnOpeningURL(
		wxHtmlURLType type, const wxString& url, wxString* redirect) const override;
	HtmlViewStatus* GetStatus();
	bool GetExtDownload();
	bool SetExtDownload(bool is_allowed);
};
