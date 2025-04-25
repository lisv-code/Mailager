#include "MailMsgViewMgr.h"
#include "MailMsgEditor/MailMsgEditor.h"
#include "MailMsgViewer/MailMsgViewer.h"

namespace MailMsgViewMgr_Imp
{
	template <typename TViewWnd>
	static TViewWnd* get_emb_view_wnd(wxWindow* parent);
}
using namespace MailMsgViewMgr_Imp;

void MailMsgViewMgr::SetEmbViewDefaults(wxWindow* view_parent)
{
	embViewParent = view_parent;
}

void MailMsgViewMgr::SetStdViewDefaults(wxWindow* view_parent, ViewCreationHandler view_handler)
{
	stdViewParent = view_parent;
	viewCreationEvent = view_handler;
}

void MailMsgViewMgr::InitEmbView(std::shared_ptr<MailMsgFile> mail_msg)
{
	if (!mail_msg) {
		if (embViewParent) embViewParent->DestroyChildren();
		return;
	}

	auto msg_view = IsMailMsgEditable(mail_msg.get())
		? static_cast<MailMsgFileView*>(get_emb_view_wnd<MailMsgEditor>(embViewParent))
		: static_cast<MailMsgFileView*>(get_emb_view_wnd<MailMsgViewer>(embViewParent));

	msg_view->SetMailMsgFile(mail_msg);
}

void MailMsgViewMgr::OpenStdView(std::shared_ptr<MailMsgFile> mail_msg)
{
	MailMsgFileView* msg_view = IsMailMsgEditable(mail_msg.get())
		? static_cast<MailMsgFileView*>(new MailMsgEditor(stdViewParent))
		: static_cast<MailMsgFileView*>(new MailMsgViewer(stdViewParent));

	msg_view->SetMailMsgFile(mail_msg);

	viewCreationEvent(dynamic_cast<wxWindow*>(msg_view),
		MailMsgViewMgr::IsMailMsgEditable(mail_msg.get())
			? MailMsgEditor_Def::WndTitle : MailMsgViewer_Def::WndTitle);
}

bool MailMsgViewMgr::IsMailMsgEditable(MailMsgFile* mail_msg)
{
	if (mail_msg) {
		if (mail_msg->GetFilePath() == nullptr) return true;
		auto msg_status = mail_msg->GetStatus();
		return (MailMsgStatus::mmsIsDraft & msg_status)
			&& !(MailMsgStatus::mmsIsSent & msg_status);
	}
	return true;
}

template<typename TViewWnd>
TViewWnd* MailMsgViewMgr_Imp::get_emb_view_wnd(wxWindow* parent)
{
	TViewWnd* msg_view = nullptr;
	if (parent) {
		auto children = parent->GetChildren();
		if (children.GetCount()) {
			for (auto child : children) {
				msg_view = dynamic_cast<TViewWnd*>(child);
				if (msg_view) break;
			}
		}
	}
	if (!msg_view) {
		if (parent) parent->DestroyChildren();
		msg_view = new TViewWnd(parent);
		if (parent) {
			auto sizer = parent->GetSizer();
			if (!sizer) sizer = new wxBoxSizer(wxVERTICAL);
			sizer->Clear();
			sizer->Add(msg_view, 1, wxALL | wxEXPAND, 0);
			if (!parent->GetSizer()) parent->SetSizer(sizer);
			parent->Layout();
		}
	}
	return msg_view;
}
