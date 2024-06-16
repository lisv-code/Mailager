#include "UiHelper.h"
#include <wx/artprov.h>
#include <LisCommon/StrUtils.h>
#include "ResMgr.h"

class ArtProviderX1 : public wxArtProvider {
protected:
	virtual wxBitmap CreateBitmap(const wxArtID& id, const wxArtClient& client, const wxSize& size) override
	{
		if (client == wxART_OTHER)
		{
			if (id.StartsWith("IcoTool"))
				return ResMgr::GetIcon(id, 16, 16);
			if (id.StartsWith("IcoBtn"))
				return ResMgr::GetIcon(id, 19, 19);

			return ResMgr::GetIcon(id, -1, -1);
		};
		return wxNullBitmap;
	};
};

UiHelper::UiHelper() { }

UiHelper::~UiHelper() { }

void UiHelper::Init()
{
	wxArtProvider::Push(new ArtProviderX1());
}

void UiHelper::InitDialog(wxTopLevelWindowBase* window)
{
	window->SetIcon(ResMgr::GetIcon(_T("IcoAppMain")));
}
