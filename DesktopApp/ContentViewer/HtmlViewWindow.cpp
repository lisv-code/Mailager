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
#define UriScheme_Data "data"
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
		wxString norm_url(url);
		norm_url.Trim(true).Trim(false); // Normalize the URL. TODO: ? maybe consecutive dots should be fixed
		if (norm_url.Left(schemePrefix.size()).IsSameAs(schemePrefix, false)) {
			// URL has the scheme prefix: allow to process by default handler
			result = wxHTML_OPEN;
		} else {
			auto uri_scheme = wxURI(norm_url).GetScheme();
			if (LisStr::StrIStr(uri_scheme, _T(UriScheme_Data))) {
				// DATA scheme: allow to process by default handler
				result = wxHTML_OPEN;
			} else if (LisStr::StrIStr(uri_scheme, _T(UriScheme_ContentId))) {
				// CID scheme: insert the scheme prefix and redirect to further processing
				(*redirect) = schemePrefix + norm_url;
				result = wxHTML_REDIRECT;
			} else if (UrlSchemeHandler_Inet::CanOpenUrl(norm_url)) {
				// I-net URL: allow to process if downloaded, else add to the list
				// TODO: detect tracking links and omit them (? introduce server blacklist)
				if (isExtDownload || ExtResMgr::GetInstance()->GetResourceData(norm_url, nullptr, true))
					result = wxHTML_OPEN;
				else status.ExternalImages.push_back(norm_url);
			}
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
