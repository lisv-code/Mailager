#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif
#include <wx/treectrl.h>

class UiHelper {
	UiHelper();
	~UiHelper();
public:
	static void Init();

	static void InitDialog(wxTopLevelWindowBase* window);
};
