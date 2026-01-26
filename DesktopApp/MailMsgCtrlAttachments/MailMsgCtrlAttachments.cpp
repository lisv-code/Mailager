#include "MailMsgCtrlAttachments.h"
#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include "../../CoreAppLib/AppDef.h"
#include "../../CoreMailLib/MimeNodeRead.h"
#include "../../CoreMailLib/MimeNodeWrite.h"

namespace MailMsgCtrlAttachments_Imp
{
	const TCHAR* MnuTitle_SaveFile = _T("Save As...");
	const TCHAR* MnuTitle_Remove = _T("Remove...");
	const TCHAR* DlgTitle_AddAttachment = _T("Add attachment");
	const TCHAR* DlgTitle_SaveAttachment = _T("Save attachment");
	const TCHAR* Msg_RemoveAttachmentQuestion = _T("Remove attachment?\n\n%s");
	const TCHAR* Msg_ErrorSavingAttachment = _T("Error saving attachment to file. \n\n%s");
	const TCHAR* Msg_ErrorLoadingAttachment = _T("Error loading data for attachment. \n\n%s");
}
using namespace MailMsgCtrlAttachments_Imp;

MailMsgCtrlAttachments::MailMsgCtrlAttachments(wxWindow* wnd_parent, wxWindow* wnd_container, bool allow_edit)
	: wndParent(wnd_parent), wndContainer(wnd_container), mnuAttachmentFile()
{
	InitItemMenu(allow_edit);
}

MailMsgCtrlAttachments::~MailMsgCtrlAttachments()
{
	if (wndContainer) wndContainer->DestroyChildren();
	wndParent = wndContainer = nullptr;
	for (auto node : own_nodes) {
		if (node) delete node;
	}
	own_nodes.clear();
}

void MailMsgCtrlAttachments::InitItemMenu(bool allow_edit)
{
	if (mnuAttachmentFile.GetMenuItemCount() > 0) { // clean up
		auto items = mnuAttachmentFile.GetMenuItems();
		for (auto item : items) mnuAttachmentFile.Destroy(item);
	}
	auto mnu_item = mnuAttachmentFile.Append(wxID_ANY, MnuTitle_SaveFile);
	mnuAttachmentFile.Bind(wxEVT_MENU, &MailMsgCtrlAttachments::SaveAttachmentFile_EventHandler,
		this, mnu_item->GetId());
	if (allow_edit) {
		mnu_item = mnuAttachmentFile.Append(wxID_ANY, MnuTitle_Remove);
		mnuAttachmentFile.Bind(wxEVT_MENU, &MailMsgCtrlAttachments::RemoveAttachment_EventHandler,
			this, mnu_item->GetId());
	}
}

void MailMsgCtrlAttachments::SetMode(bool allow_edit)
{
	InitItemMenu(allow_edit);
}

void MailMsgCtrlAttachments::LoadAttachments(const MimeNode& node, bool take_ownership)
{
	wndContainer->DestroyChildren();
	std::vector<MimeNode*> loaded_nodes;
	int result = const_cast<MimeNode&>(node).EnumDataStructure([this, &loaded_nodes](MimeNode* entity)
	{
		auto node_type = MimeNodeRead::get_node_content_flags(entity);
		if (MimeNodeContentFlags::ncfIsAttachment & node_type) {
			if (AddAttachment(entity, false, false)) loaded_nodes.push_back(entity);
		}
		return 0;
	});
	wndContainer->Show(wndContainer->GetChildren().GetCount());
	wndContainer->GetParent()->Layout();
	if (take_ownership) TakeDataOwnership(loaded_nodes.data(), loaded_nodes.size());
}

void MailMsgCtrlAttachments::AddAttachmentFileButton(const wxString& name, MimeNode* data_node)
{
	auto btn = new wxButton(wndContainer, wxID_ANY, name);
	btn->SetClientData((void*)data_node);
	btn->Bind(wxEVT_BUTTON, &MailMsgCtrlAttachments::AttachmentFileButton_ClickHandler, this);
	btn->Bind(wxEVT_CONTEXT_MENU, &MailMsgCtrlAttachments::AttachmentFileButton_ClickHandler, this);
	wndContainer->GetSizer()->Add(btn);
}

void MailMsgCtrlAttachments::TakeDataOwnership(MimeNode** data_nodes, size_t count)
{
	for (size_t i = 0; i < count; ++i) {
		own_nodes.push_back(data_nodes[i]);
		if (data_nodes[i]->GetParent())
			data_nodes[i]->GetParent()->RemovePart(data_nodes[i], true);
	}
}

bool MailMsgCtrlAttachments::AddAttachment(MimeNode* data_node, bool take_ownership, bool refresh_view)
{
	if (!data_node) return false;
	std::basic_string<TCHAR> name;
	MimeNodeRead::read_file_name(data_node, name);
	AddAttachmentFileButton(name, data_node);
	if (take_ownership) TakeDataOwnership(&data_node, 1);
	if (refresh_view) {
		wndContainer->Show();
		wndContainer->GetParent()->Layout();
	}
	return true;
}

bool MailMsgCtrlAttachments::Dialog_NewAttachment()
{
	wxFileDialog dlg(wndParent, DlgTitle_AddAttachment, "", "", "", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (wxID_OK == dlg.ShowModal()) {
		MimeNode* data_node = new MimeNode();
		wxBeginBusyCursor();
		int result = MimeNodeWrite::load_data_content_bin(*data_node, dlg.GetPath());
		wxEndBusyCursor();
		if (result >= 0) {
			MimeNodeWrite::set_data_node_header(*data_node, dlg.GetFilename(), false);
			return AddAttachment(data_node, true, true);
		} else {
			delete data_node;
			wxMessageBox(wxString::Format(Msg_ErrorLoadingAttachment, dlg.GetPath()),
				AppDef_Title, wxICON_ERROR | wxOK, wndParent);
		}
	}
	return false;
}

MailMsgCtrlAttachments::NodeList MailMsgCtrlAttachments::GetAttachments(bool pass_ownership)
{
	NodeList result;
	const auto children = wndContainer->GetChildren();
	for (const auto node : children) {
		wxButton* btn = wxDynamicCast(node, wxButton);
		if (btn) {
			MimeNode* data_node = (MimeNode*)btn->GetClientData();
			if (data_node) {
				result.push_back(data_node);
				if (pass_ownership)
					own_nodes.erase(
						std::remove(own_nodes.begin(), own_nodes.end(), data_node), own_nodes.end());
			}
		}
	}
	return result;
}

void MailMsgCtrlAttachments::RemoveAttachment_EventHandler(wxCommandEvent& event)
{
	wxMenu* mnu = wxDynamicCast(event.GetEventObject(), wxMenu);
	wxButton* btn = (wxButton*)(mnu->GetClientData());
	if (wxOK == wxMessageBox(wxString::Format(Msg_RemoveAttachmentQuestion, btn->GetLabelText()), AppDef_Title,
		wxICON_QUESTION | wxOK | wxCANCEL | wxCANCEL_DEFAULT, wndParent))
	{
		MimeNode* data_node = (MimeNode*)btn->GetClientData();
		if (data_node) {
			auto own_node_item = std::find(own_nodes.begin(), own_nodes.end(), data_node);
			if (own_node_item != own_nodes.end()) {
				own_nodes.erase(own_node_item);
				delete data_node;
			}
		}
		btn->SetClientData(nullptr);
		btn->Destroy();
	}
}

void MailMsgCtrlAttachments::SaveAttachmentFile_EventHandler(wxCommandEvent& event)
{
	wxMenu* mnu = wxDynamicCast(event.GetEventObject(), wxMenu);
	wxButton* btn = (wxButton*)(mnu->GetClientData());
	wxFileDialog dlg(
		wndParent, DlgTitle_SaveAttachment, "", btn->GetLabelText(), "", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (wxID_OK == dlg.ShowModal()) {
		MimeNode* data_node = (MimeNode*)btn->GetClientData();
		wxBeginBusyCursor();
		int result = MimeNodeRead::save_content_data_bin(data_node, dlg.GetPath());
		wxEndBusyCursor();
		if (0 > result)
			wxMessageBox(wxString::Format(Msg_ErrorSavingAttachment, dlg.GetPath()),
				AppDef_Title, wxICON_ERROR | wxOK, wndParent);
	}
}

void MailMsgCtrlAttachments::AttachmentFileButton_ClickHandler(wxCommandEvent& event)
{
	wxButton* btn = wxDynamicCast(event.GetEventObject(), wxButton);
	mnuAttachmentFile.SetClientData((void*)btn);
	btn->PopupMenu(&mnuAttachmentFile);
}
