#include "MailMsgViewer.h"
#include <sstream>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include "../../CoreAppLib/AppDef.h"
#include "../../CoreMailLib/MimeParser.h"
#include "../../CoreMailLib/MimeMessageDef.h"
#include "../AppCfg.h"
#include "../ContentViewer/HtmlContentViewer.h"
#include "../ContentViewer/WebContentViewer.h"
#include "../SysHelper.h"

namespace MailMsgViewer_Def
{
	const TCHAR* WndTitle = _T("Mail view");
}
using namespace MailMsgViewer_Def;
namespace MailMsgViewer_Imp
{
#define MsgTimeFailView "<???>"
#define HtmlViewPrefix "<html><body>"
#define HtmlViewSuffix "</body></html>"
	const TCHAR* Msg_SaveMsgContent = _T("Save message content");
	const TCHAR* Msg_ErrorSavingMsgContent = _T("Error saving message content");
	const TCHAR* Msg_ErrorOpeningMsgExt = _T("Error opening message externally");
	const TCHAR* Msg_SaveAttachment = _T("Save attachment");
	const TCHAR* Msg_ErrorSavingAttachment = _T("Error saving attachment file");
}
using namespace MailMsgViewer_Imp;

MailMsgViewer::MailMsgViewer(wxWindow* parent) : MailMsgViewerUI(parent)
{
	InitContentViewer(AppCfg.Get().MailMessageContentViewer);
}

MailMsgViewer::~MailMsgViewer()
{
	if (contentViewer) delete contentViewer;
}

int MailMsgViewer::OnMailMsgFileSet()
{
	structInfo.clear();
	std::string struct_descr;
	int struct_result = LoadData(struct_descr);
	structInfo = struct_descr;
	structInfo += "(items: " + std::to_string(struct_result) + ")";

	RefreshHeaderView();

	contentViewer->SetExtResourcesDownload(false);
	RefreshView(true);

	return 0;
}

void MailMsgViewer::toolSwitchContentView_OnToolClicked(wxCommandEvent& event)
{
	isViewMsgStruct = !isViewMsgStruct;
	RefreshView(true);
}

void MailMsgViewer::toolSaveContent_OnToolClicked(wxCommandEvent& event)
{
	auto view_node = FindRootViewNode();
	if (!view_node) {
		wxMessageBox(Msg_ErrorSavingMsgContent, AppDef_Title, wxICON_ERROR | wxOK);
		return;
	}
	std::string data_type;
	MimeNodeProc::GetNodeType(view_node, &data_type);
	wxString file_name;
	if (std::string::npos != data_type.find("html")) file_name = "message.htm";
	else if (std::string::npos != data_type.find("plain")) file_name = "message.txt";
	else file_name = "";
	wxFileDialog dlg(this, Msg_SaveMsgContent, "", file_name, "", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (wxID_OK == dlg.ShowModal()) {
		if (0 > MimeNodeProc::SaveContentDataTxt(view_node, dlg.GetPath()))
			wxMessageBox(Msg_ErrorSavingMsgContent, AppDef_Title, wxICON_ERROR | wxOK);
	}
}

void MailMsgViewer::toolOpenMessage_OnToolClicked(wxCommandEvent& event)
{
	if (!SysHelper::Open(mailMsgFile->GetFilePath())) {
		wxMessageBox(Msg_ErrorOpeningMsgExt, AppDef_Title, wxICON_ERROR | wxOK);
	}
}

void MailMsgViewer::mnuAttachmentFileSave_OnMenuSelection(wxCommandEvent& event)
{
	wxMenu *mnu = wxDynamicCast(event.GetEventObject(), wxMenu);
	wxButton *btn = (wxButton*)(mnu->GetClientData());
	wxFileDialog dlg(
		this, Msg_SaveAttachment, "", btn->GetLabelText(), "", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (wxID_OK == dlg.ShowModal()) {
		MimeNode *mail_node = (MimeNode*)btn->GetClientData();
		if (0 > MimeNodeProc::SaveContentDataBin(mail_node, dlg.GetPath()))
			wxMessageBox(Msg_ErrorSavingAttachment, AppDef_Title, wxICON_ERROR | wxOK);
	}
}

void MailMsgViewer::InitContentViewer(int content_viewer_type)
{
	wxWindow *view_ctrl = nullptr;
	if (0 != content_viewer_type) {
		contentViewer = new WebContentViewer();
		view_ctrl = contentViewer->InitView(this, pnlContentViewerPlaceholder->GetParent());
		if (!view_ctrl) delete contentViewer;
	}
	if (!view_ctrl) {
		contentViewer = new HtmlContentViewer();
		view_ctrl = contentViewer->InitView(this, pnlContentViewerPlaceholder->GetParent());
	}
	if (view_ctrl) {
		pnlContentViewerPlaceholder->GetContainingSizer()->Add(view_ctrl, 1, wxALL | wxEXPAND, 2);
		pnlContentViewerPlaceholder->Destroy(); // Free unneeded resources
		pnlContentViewerPlaceholder = nullptr;

		contentViewer->SetContentDataProvider(
			std::bind(&MailMsgViewer::ContentEmbeddedDataProvider, this, std::placeholders::_1));
	}
}

int MailMsgViewer::LoadData(std::string& out_info)
{
	nodeStruct.clear();
	int result = mailMsgFile->LoadData(msgNode);
	if (result >= 0) {
		result = MimeNodeProc::GetNodeStructInfo(msgNode, nodeStruct, &out_info);
	}
	return result;
}

void MailMsgViewer::RefreshHeaderView()
{
	// Update header values
	txtSubject->SetValue(msgNode.Header.GetField(MailMsgHdrName_Subj).GetText());
	auto msg_time = msgNode.Header.GetField(MailMsgHdrName_Date).GetTime();
	txtDate->SetValue(msg_time ? wxDateTime(*msg_time).Format() : MsgTimeFailView);
	txtSender->SetValue(msgNode.Header.GetField(MailMsgHdrName_From).GetText());

	// Refresh header layout
	auto txt1 = new wxStaticText(txtDate->GetParent(), wxID_ANY, txtDate->GetValue());
	txtDate->SetMinClientSize(wxSize(txt1->GetSize().x + 8, txt1->GetSize().y + 6));
	txtDate->GetParent()->Layout();
	delete txt1;
}

MimeNode* MailMsgViewer::FindRootViewNode()
{
	MimeNode* result = nullptr;
	for (auto& node_info : nodeStruct) {
		if (MimeNodeProc::nctRootView & node_info.type)
			result = node_info.node;
	}
	return result;
}

wxString MailMsgViewer::ComposeStructViewContent(const wxString& struct_info, const FILE_PATH_CHAR* msg_file_path)
{
	wxString result;
	std::ifstream ifs(msg_file_path, std::ios::in | std::ios::binary);
	std::string msg_header((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
	ifs.close();
	size_t hdr_end_pos = msg_header.find((MimeMessageLineEnd MimeMessageLineEnd), 0, 4);
	if (std::string::npos != hdr_end_pos) msg_header.resize(hdr_end_pos);

	result = HtmlViewPrefix;
	auto hdr_info = wxString(msg_header);
	hdr_info.Replace("<", "&lt;");
	hdr_info.Replace(">", "&gt;");
	result += "<pre>\n" + hdr_info + "\n</pre>";
	result += "<pre>\n" + struct_info + "\n</pre>";
	result += "<p>";
	result += msg_file_path;
	result += "</p>";
	result += HtmlViewSuffix;

	return result;
}

wxString MailMsgViewer::ComposeTextViewContent(const MimeNode* node)
{
	wxString result;
	if (node) {
		std::basic_string<TCHAR> node_data;
		int res_code = MimeNodeProc::GetContentDataTxt(node, node_data);
		result = node_data;
		if (0 == res_code) {
			result = _T(HtmlViewPrefix "<pre>") + result + _T("</pre>" HtmlViewSuffix);
		}
	} else {
		result = _T(HtmlViewPrefix " " HtmlViewSuffix); // No viewable message body
	}
	return result;
}

ContentViewer::ContentData MailMsgViewer::ContentEmbeddedDataProvider(const TCHAR* id)
{
	ContentViewer::ContentData result { std::string(), nullptr };
	for (const auto& item : nodeStruct) {
		if (MimeNodeProc::MimeNodeContentType::nctHasContentId & item.type) {
			std::basic_string<TCHAR> content_id;
			if (MimeNodeProc::ReadContentId(item.node, content_id) && (content_id == id)) {
				std::string type;
				std::stringstream data(std::ios::in | std::ios::out | std::ios::binary);
				if (MimeNodeProc::GetContentDataBin(item.node, type, data) >= 0) {
					result.type = type;
					result.data = new std::stringstream();
					static_cast<std::stringstream*>(result.data)->swap(data);
				}
				break;
			}
		}
	}
	return result;
}

int MailMsgViewer::RefreshView(bool update_content)
{
	InitAttachmentView();

	if (update_content)
		contentViewer->SetContent(isViewMsgStruct
			? ComposeStructViewContent(structInfo, mailMsgFile->GetFilePath())
			: ComposeTextViewContent(FindRootViewNode()));
	else
		contentViewer->ReloadContent();

	pnlExtDownload->Show(contentViewer->GetHasExternalImages()
		&& !contentViewer->GetExtResourcesDownload());
	pnlExtDownload->GetParent()->Layout();

	return 0;
}

void MailMsgViewer::InitAttachmentView()
{
	pnlAttachments->DestroyChildren();
	auto sizer = pnlAttachments->GetSizer();
	for (const auto& item : nodeStruct) {
		if (MimeNodeProc::MimeNodeContentType::nctIsAttachment & item.type) {
			std::basic_string<TCHAR> name;
			MimeNodeProc::ReadFileName(item.node, name);
			auto btn = new wxButton(pnlAttachments, wxID_ANY, name);
			btn->Bind(wxEVT_BUTTON, &MailMsgViewer::btnAttachmentFile_Clicked, this);
			btn->SetClientData((void*)item.node);
			sizer->Add(btn);
		}
	}
	pnlAttachments->Show(pnlAttachments->GetChildren().GetCount());
}

void MailMsgViewer::btnAttachmentFile_Clicked(wxCommandEvent& event)
{
	wxButton* btn = wxDynamicCast(event.GetEventObject(), wxButton);
	mnuAttachmentFile->SetClientData((void*)btn);
	btn->PopupMenu(mnuAttachmentFile);
}

void MailMsgViewer::btnDownloadImages_OnButtonClick(wxCommandEvent& event)
{
	if (contentViewer && contentViewer->SetExtResourcesDownload(true)) {
		wxBeginBusyCursor();
		RefreshView(false);
		wxEndBusyCursor();
	}
}
