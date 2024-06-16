#include "HtmlViewWindow.h"
#include <wx/msgdlg.h>
#include <wx/uri.h>
#include <LisCommon/StrUtils.h>
#include "../../CoreAppLib/AppDef.h"
#include "../SysHelper.h"
#include "ExtResMgr.h"
#include "UrlSchemeHandler_Cid.h"
#include "UrlSchemeHandler_Inet.h"

namespace HtmlViewWindow_Imp
{
#define Log_Scope "HtmlView"
	const wxChar* Msg_OpenUrlQuestion = _T("Open URL?\n\n%s");
}
using namespace HtmlViewWindow_Imp;

HtmlViewWindow::HtmlViewWindow(wxWindow* parent, wxWindowID id, const char* scheme_prefix)
	: wxHtmlWindow(parent, id)
{
	isExtDownload = false;
	schemePrefix = scheme_prefix;
}

void HtmlViewWindow::OnLinkClicked(const wxHtmlLinkInfo& link)
{
	// wxHtmlWindow::OnLinkClicked(link);
	if (wxOK == wxMessageBox(wxString::Format(Msg_OpenUrlQuestion, link.GetHref()), AppDef_Title,
			wxICON_QUESTION | wxOK | wxCANCEL | wxCANCEL_DEFAULT, this->GetParent()))
	{
#ifdef _WINDOWS
		SysHelper::Open((const TCHAR*)(link.GetHref()));
#else
		SysHelper::Open(link.GetHref());
#endif
	}
}

wxHtmlOpeningStatus HtmlViewWindow::OnOpeningURL(
	wxHtmlURLType type, const wxString& url, wxString* redirect) const
{
	// return wxHtmlWindow::OnOpeningURL(type, url, redirect);
	auto result = wxHTML_BLOCK;
	logger->LogFmt(LisLog::llTrace, Log_Scope " opening URL: %i %s", (int)type, (char*)url.char_str());
	if (wxHTML_URL_IMAGE == type) {
		wxString urlX(url);
		urlX.Trim(true).Trim(false); // Normalize the URL. TODO: ? maybe consecutive dots should be fixed
		if (urlX.Left(schemePrefix.size()).IsSameAs(schemePrefix, false)) {
			result = wxHTML_OPEN;
		} else if (LisStr::StrIStr(wxURI(urlX).GetScheme(), _T(UriScheme_ContentId))) {
			(*redirect) = schemePrefix + urlX;
			result = wxHTML_REDIRECT;
		} else if (UrlSchemeHandler_Inet::CanOpenUrl(urlX)) {
			// TODO: detect tracking links and skip them (? server blacklist)
			if (isExtDownload || ExtResMgr::GetInstance()->GetResourceData(urlX, nullptr, true))
				result = wxHTML_OPEN;
			else status.ExternalImages.push_back(urlX);
		}
	}
	return result;
}

HtmlViewStatus* HtmlViewWindow::GetStatus()
{
	return &status;
}

bool HtmlViewWindow::GetExtDownload()
{
	return isExtDownload;
}

bool HtmlViewWindow::SetExtDownload(bool is_allowed)
{
	isExtDownload = is_allowed;
	return true;
}
