#include "WebContentViewer.h"
#include <wx/msgdlg.h>
#include <wx/uri.h>
#ifdef _WINDOWS
#include <wx/msw/webview_ie.h>
#else
// #include <wx/gtk/webview_webkit.h> // <wx/osx/webview_webkit.h>
#endif
#include "../../CoreAppLib/AppDef.h"
#include "../SysHelper.h"
#include "WebViewHandler_Cid.h"
#include "WebViewHandler_Null.h"

namespace WebContentViewer_Imp
{
#define Log_Scope "WebView"
	const wxChar* Msg_OpenUrlQuestion = _T("Open URL?\n\n%s");
}
using namespace WebContentViewer_Imp;

WebContentViewer::WebContentViewer()
{
	viewCtrl = nullptr;
	hasExternalImages = false;
}

WebContentViewer::~WebContentViewer()
{
	if (viewCtrl) { delete viewCtrl; viewCtrl = nullptr; }
}

wxWindow* WebContentViewer::InitView(wxWindow* parent, wxWindow* container)
{
	// MSW: the default backend is wxWEBVIEW_BACKEND_IE.
	// _ wxWEBVIEW_BACKEND_EDGE can be used (see wxWebView documentation)
	// GTK: the default backend is wxWEBVIEW_WEBKIT
	// _ libwebkitgtk-dev package 1.3.1+ is required (ships with Ubuntu starting from 11.04 (Natty))
	// _ GTK3 - WebKit2 version is used (libwebkit2gtk-4.0-dev)

	static bool is_1st_init = true;
	if (is_1st_init) {
		auto version_info = wxWebView::GetBackendVersionInfo();
		logger->LogFmt(LisLog::llInfo, Log_Scope " default backend: %s.",
			(char*)version_info.GetVersionString().char_str());
	}

	if (!viewCtrl) {
		viewCtrl = wxWebView::New(container, wxID_ANY);
		if (viewCtrl) {
			if (is_1st_init) {
#ifdef _WINDOWS
				if (wxWebView::IsBackendAvailable(wxWebViewBackendIE)) {
					auto ie_view = dynamic_cast<wxWebViewIE*>(viewCtrl);
					if (ie_view) ie_view->MSWSetEmulationLevel(wxWEBVIEWIE_EMU_IE11);
				}
#endif
			}

			viewCtrl->EnableContextMenu(false);
			viewCtrl->EnableAccessToDevTools(false);
			viewCtrl->EnableHistory(false);

			parent->Bind(wxEVT_WEBVIEW_LOADED, &WebContentViewer::OnWebViewLoaded, this, viewCtrl->GetId());
			parent->Bind(wxEVT_WEBVIEW_ERROR, &WebContentViewer::OnWebViewError, this, viewCtrl->GetId());
			parent->Bind(wxEVT_WEBVIEW_NAVIGATING, &WebContentViewer::OnWebViewNavigating, this, viewCtrl->GetId());
			parent->Bind(wxEVT_WEBVIEW_NEWWINDOW, &WebContentViewer::OnWebViewNewWindow, this, viewCtrl->GetId());

			// TODO: Likely, user should decide if external resources can be loaded,
			// but now try to disable any external content load
			// WARNING: on MacOS RegisterHandler has to be called before Create (two-step creation is required)
			viewCtrl->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new WebViewHandler_Null("http", &hasExternalImages)));
			viewCtrl->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new WebViewHandler_Null("https", &hasExternalImages)));
			viewCtrl->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new WebViewHandler_Null("file", &hasExternalImages)));
			viewCtrl->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new WebViewHandler_Null("ftp", &hasExternalImages)));
		}
		else {
			logger->LogTxt(LisLog::llError, Log_Scope " intialization failed.");
		}
	}

	if (is_1st_init) is_1st_init = false;

	return viewCtrl;
}

bool WebContentViewer::SetContentDataProvider(ContentDataProvider proc)
{
	if (viewCtrl) {
		// WARNING: on MacOS RegisterHandler has to be called before Create
		viewCtrl->RegisterHandler(
			wxSharedPtr<wxWebViewHandler>(new WebViewHandler_Cid(UriScheme_ContentId, proc)));
		return true;
	}
	return false;
}

bool WebContentViewer::SetContent(const wxChar* content)
{
	hasExternalImages = false;
	if (viewCtrl) viewCtrl->SetPage(content, "");
	return viewCtrl != nullptr;
}

bool WebContentViewer::ReloadContent()
{
	if (viewCtrl) {
		viewCtrl->Reload();
		return true;
	}
	return false;
}

bool WebContentViewer::GetHasExternalImages()
{
	return hasExternalImages;
}

void WebContentViewer::OpenUrl(const wxString& url)
{
	if (wxOK == wxMessageBox(wxString::Format(Msg_OpenUrlQuestion, url), AppDef_Title,
		wxICON_QUESTION | wxOK | wxCANCEL | wxCANCEL_DEFAULT, viewCtrl->GetParent()))
	{
#ifdef _WINDOWS
		SysHelper::Open((const TCHAR*)url);
#else
		SysHelper::Open(url);
#endif
	}
}

void WebContentViewer::OnWebViewLoaded(wxWebViewEvent& event)
{
	logger->LogTxt(LisLog::llTrace, Log_Scope " document loaded.");
}

void WebContentViewer::OnWebViewError(wxWebViewEvent& event)
{
	logger->LogFmt(LisLog::llError, Log_Scope " ERROR. url: %s\r\n%s",
		(char*)event.GetURL().char_str(), (char*)event.GetString().char_str());
}

void WebContentViewer::OnWebViewNavigating(wxWebViewEvent& event)
{
	wxURI uri(event.GetURL());
	if (!uri.GetScheme().IsSameAs("about")) { // Scheme "about:" (the local window) allowed by default
		OpenUrl(event.GetURL());
		event.Veto();
	}
}

void WebContentViewer::OnWebViewNewWindow(wxWebViewEvent& event)
{
#ifdef _WINDOWS
	if (wxWEBVIEW_NAV_ACTION_USER == event.GetNavigationAction())
#endif
	{
		OpenUrl(event.GetURL());
	}
	event.Veto();
}
