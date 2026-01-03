#pragma once
#include <wx/menu.h>
#include <wx/window.h>
#include "../../CoreMailLib/MimeNodeRead.h"

class MailMsgCtrlAttachments
{
public:
	typedef std::vector<MimeNode*> NodeList;
private:
	NodeList own_nodes;
	wxWindow* wndParent, *wndContainer;
	wxMenu mnuAttachmentFile;

	void AddAttachmentFileButton(const wxString& name, MimeNode* msg_node);
	void AttachmentFileButton_ClickHandler(wxCommandEvent& event);
	void RemoveAttachment_EventHandler(wxCommandEvent& event);
	void SaveAttachmentFile_EventHandler(wxCommandEvent& event);
public:
	MailMsgCtrlAttachments(wxWindow* wnd_parent, wxWindow* wnd_container, bool allow_edit);
	~MailMsgCtrlAttachments();

	void InitItemMenu(bool allow_edit);
	void LoadAttachments(const MimeNodeRead::NodeInfoContainer& node_struct);
	bool AddAttachment(MimeNode* data_node, bool take_ownership, bool refresh_view);
	bool Dialog_NewAttachment();
	NodeList GetAttachments(bool pass_ownership);
};
