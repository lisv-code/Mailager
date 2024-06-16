#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

class ResMgr {
	ResMgr();
	~ResMgr();
public:
	static const int VersionInfoFieldLen = 0xFF;
	struct VersionInfo {
		wxChar Version[VersionInfoFieldLen], Copyright[VersionInfoFieldLen], Comments[VersionInfoFieldLen];
	};
	static VersionInfo GetVersionInfo();

	static wxIcon GetIcon(const wxString& resourceName, int width = -1, int height = -1);
};
