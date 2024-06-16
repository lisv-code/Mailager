#pragma once
#include <memory>
#include <wx/window.h>
#include "../CoreAppLib/MailMsgFile.h"

class MailMsgViewMgr
{
public:
	typedef std::function<void(wxWindow* wnd, const TCHAR* title)> ViewCreationHandler;
private:
	wxWindow *embViewParent, *stdViewParent;
	ViewCreationHandler viewCreationEvent;
	static bool IsMailMsgEditable(const MailMsgFile* mail_msg);
public:
	void SetEmbViewDefaults(wxWindow* view_parent);
	void SetStdViewDefaults(wxWindow* view_parent, ViewCreationHandler view_handler);
	void InitEmbView(std::shared_ptr<MailMsgFile> mail_msg); // Embedded (preview)
	void OpenStdView(std::shared_ptr<MailMsgFile> mail_msg); // Standalone (tab)
};
