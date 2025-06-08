#include "MailMsgViewMgr.h"
#include "MailMsgEditor/MailMsgEditor.h"
#include "MailMsgViewer/MailMsgViewer.h"

namespace MailMsgViewMgr_Imp
{
#define MaxStdViewCount 0xFFFF

	static bool is_mail_msg_editable(MailMsgFile* mail_msg);
	static int find_std_msg_view(IMailMsgViewCtrl* view_ctrl, MailMsgFile* mail_msg, bool activate);

	template <typename TViewWnd> static TViewWnd* find_emb_view_wnd(wxWindow* parent);
	template <typename TViewWnd> static TViewWnd* get_emb_view_wnd(wxWindow* parent);
}
using namespace MailMsgViewMgr_Imp;

void MailMsgViewMgr::SetEmbViewDefaults(wxWindow* view_parent)
{
	embViewParent = view_parent;
}

void MailMsgViewMgr::SetStdViewDefaults(wxWindow* view_parent, IMailMsgViewCtrl* view_ctrl)
{
	stdViewParent = view_parent;
	mailMsgViewCtrl = view_ctrl;
}

void MailMsgViewMgr::InitEmbView(std::shared_ptr<MailMsgFile> mail_msg)
{
	if (!mail_msg) {
		if (embViewParent) embViewParent->DestroyChildren();
		return;
	}

	bool is_editable = is_mail_msg_editable(mail_msg.get());
	auto msg_view = is_editable
		? static_cast<MailMsgFileView*>(get_emb_view_wnd<MailMsgEditor>(embViewParent))
		: static_cast<MailMsgFileView*>(get_emb_view_wnd<MailMsgViewer>(embViewParent));

	msg_view->SetMailMsgFile(mail_msg);
	if (is_editable && find_std_msg_view(mailMsgViewCtrl, mail_msg.get(), false) >= 0)
		msg_view->SetCanEdit(false); // Don't allow embedded editing if another editor is already active
}

void MailMsgViewMgr::OpenStdView(std::shared_ptr<MailMsgFile> mail_msg)
{
	if (find_std_msg_view(mailMsgViewCtrl, mail_msg.get(), true) >= 0) return;

	bool is_editable = is_mail_msg_editable(mail_msg.get());

	if (is_editable) {
		auto emb_view = find_emb_view_wnd<MailMsgFileView>(embViewParent);
		if (emb_view && (emb_view->GetMailMsgFile() == mail_msg.get())) {
			emb_view->SetCanEdit(false); // Deactivate embedded editing
			// TODO: ? move changes (if they are) to the new editor, maybe warn user
		}
	}

	MailMsgFileView* msg_view = is_editable
		? static_cast<MailMsgFileView*>(new MailMsgEditor(stdViewParent))
		: static_cast<MailMsgFileView*>(new MailMsgViewer(stdViewParent));

	msg_view->SetMailMsgFile(mail_msg);

	mailMsgViewCtrl->OnViewCreated(dynamic_cast<wxWindow*>(msg_view),
		is_editable ? MailMsgEditor_Def::WndTitle : MailMsgViewer_Def::WndTitle);
}

// ************************************** MailMsgViewMgr_Imp ***************************************

bool MailMsgViewMgr_Imp::is_mail_msg_editable(MailMsgFile* mail_msg)
{
	if (mail_msg) {
		if (mail_msg->GetFilePath() == nullptr) return true;
		auto msg_status = mail_msg->GetStatus();
		return MailMsgStatus::mmsIsDraft & msg_status;
	}
	return false; // must be an error
}

int MailMsgViewMgr_Imp::find_std_msg_view(IMailMsgViewCtrl* view_ctrl, MailMsgFile* mail_msg,
	bool activate)
{
	int result = -1;
	int view_idx = 0;
	while (view_idx < MaxStdViewCount) {
		wxWindow* view_wnd = view_ctrl->GetView(view_idx);
		if (view_wnd) {
			auto msg_view = dynamic_cast<MailMsgFileView*>(view_wnd);
			if (msg_view && (msg_view->GetMailMsgFile() == mail_msg)) {
				result = view_idx;
				if (activate) view_ctrl->ActivateView(view_idx);
				break;
			}
		} else break;
		++view_idx;
	}
	return result;
}

template<typename TViewWnd>
TViewWnd* MailMsgViewMgr_Imp::find_emb_view_wnd(wxWindow* parent)
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
	return msg_view;
}

template<typename TViewWnd>
TViewWnd* MailMsgViewMgr_Imp::get_emb_view_wnd(wxWindow* parent)
{
	TViewWnd* msg_view = find_emb_view_wnd<TViewWnd>(parent);
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
