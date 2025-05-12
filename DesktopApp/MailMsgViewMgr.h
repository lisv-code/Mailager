#pragma once
#include <memory>
#include <wx/window.h>
#include "../CoreAppLib/MailMsgFile.h"

class IMailMsgViewCtrl
{
public:
	virtual ~IMailMsgViewCtrl() {}
	virtual void OnViewCreated(wxWindow* wnd, const TCHAR* title) = 0;
	virtual wxWindow* GetView(int index) = 0;
	virtual void ActivateView(int index) = 0;
};


class MailMsgViewMgr
{
	wxWindow *embViewParent, *stdViewParent;
	IMailMsgViewCtrl *mailMsgViewCtrl;
public:
	void SetEmbViewDefaults(wxWindow* view_parent);
	void SetStdViewDefaults(wxWindow* view_parent, IMailMsgViewCtrl* view_ctrl);

	void InitEmbView(std::shared_ptr<MailMsgFile> mail_msg); // Embedded (preview)
	void OpenStdView(std::shared_ptr<MailMsgFile> mail_msg); // Standalone (tab)
};
