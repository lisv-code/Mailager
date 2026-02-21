#include "UiHelper.h"
#include <wx/artprov.h>
#include <LisCommon/StrUtils.h>
#include "ResMgr.h"

#define Res_AppMainIcon "IcoAppMain"

#define Res_ToolIconPrefix "IcoTool"
#define Res_ToolIconSideSize 16

#define Res_BtnIconPrefix "IcoBtn"
#define Res_BtnIconSideSize 19

class ArtProviderX1 : public wxArtProvider {
protected:
	virtual wxBitmap CreateBitmap(const wxArtID& id, const wxArtClient& client, const wxSize& size) override
	{
		if (client == wxART_OTHER)
		{
			if (id.StartsWith(Res_ToolIconPrefix))
				return ResMgr::GetIcon(id, Res_ToolIconSideSize, Res_ToolIconSideSize);
			if (id.StartsWith(Res_BtnIconPrefix))
				return ResMgr::GetIcon(id, Res_BtnIconSideSize, Res_BtnIconSideSize);

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
	window->SetIcon(ResMgr::GetIcon(_T(Res_AppMainIcon)));
}
