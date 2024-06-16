#include "HtmlContentViewer.h"
//#include "../../wxWidgetsX/fs_data.h"
#include <LisCommon/StrUtils.h>
#include "ExtResMgr.h"
#include "HtmlViewWindow.h"
#include "UrlSchemeHandler_Cid.h"
#include "UrlSchemeHandler_Inet.h"

namespace HtmlContentViewer_Imp
{
	HtmlViewWindow* get_wnd(void* ctrl) { return static_cast<HtmlViewWindow*>(ctrl); }
}
using namespace HtmlContentViewer_Imp;

HtmlContentViewer::HtmlContentViewer()
{
	viewCtrl = nullptr;
	cidHandler = nullptr;

	// Using prefix to separate requests of the different viewer instances
	// and handle downloads only for permitted.
	// Scheme name must start with an "alpha" symbol, may contain alphanumeric and '.', '+', '-'.
	schemePrefix += "X";
	schemePrefix += (char*)LisStr::CIntToStr((int64_t)this, 36);
	schemePrefix += "-";
}

HtmlContentViewer::~HtmlContentViewer()
{
	if (viewCtrl) {
		delete viewCtrl;
		viewCtrl = nullptr;
	}
	if (cidHandler) {
		wxFileSystem::RemoveHandler(cidHandler);
		delete cidHandler;
	}
}

wxWindow* HtmlContentViewer::InitView(wxWindow* parent, wxWindow* container)
{
	static bool is_1st_init = true;
	if (is_1st_init) {
		//wxFileSystem::AddHandler(new wxDataSchemeFSHandler);
		wxFileSystem::AddHandler(new UrlSchemeHandler_Inet);
		is_1st_init = false;
	}

	if (!viewCtrl) {
		viewCtrl = new HtmlViewWindow(container, wxID_ANY, schemePrefix.c_str());
	}
	return viewCtrl;
}

bool HtmlContentViewer::SetContentDataProvider(ContentViewer::ContentDataProvider proc)
{
	if (cidHandler) {
		wxFileSystem::RemoveHandler(cidHandler);
		delete cidHandler;
	}
	cidHandler = new UrlSchemeHandler_Cid(proc, schemePrefix.c_str());
	wxFileSystem::AddHandler(cidHandler);
	return true;
}

bool HtmlContentViewer::GetExtResourcesDownload()
{
	return get_wnd(viewCtrl)->GetExtDownload();
}

bool HtmlContentViewer::SetExtResourcesDownload(bool is_allowed)
{
	return get_wnd(viewCtrl)->SetExtDownload(is_allowed);
}

bool HtmlContentViewer::SetContent(const wxChar* content)
{
	if (viewCtrl) {
		contentCache = content;
		get_wnd(viewCtrl)->GetStatus()->Clear();
		return viewCtrl->SetPage(content);
	} else return false;
}

bool HtmlContentViewer::ReloadContent()
{
	if (viewCtrl) {
		if (GetExtResourcesDownload()) {
			for (auto& ext_img : get_wnd(viewCtrl)->GetStatus()->ExternalImages)
				ExtResMgr::GetInstance()->StartDownload(ext_img);
		}
		return viewCtrl->SetPage(contentCache);
	} else return false;
}

bool HtmlContentViewer::GetHasExternalImages()
{
	if (viewCtrl)
		return get_wnd(viewCtrl)->GetStatus()->ExternalImages.size() > 0;
	else return false;
}
