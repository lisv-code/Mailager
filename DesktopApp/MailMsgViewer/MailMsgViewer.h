#pragma once
#include <string>
#include "MailMsgViewerUI.h"
#include "../MailMsgFileView.h"
#include "../../CoreMailLib/MimeNode.h"
#include "../../CoreMailLib/MimeNodeProc.h"
#include "../ContentViewer/ContentViewer.h"

namespace MailMsgViewer_Def
{
	extern const TCHAR* WndTitle;
}

class MailMsgViewer : public MailMsgViewerUI, public MailMsgFileView
{
	ContentViewer* contentViewer;
	std::basic_string<FILE_PATH_CHAR> filePath;
	MimeNode msgNode;
	MimeNodeProc::NodeInfoContainer nodeStruct;

	bool isViewMsgStruct = false;
	wxString structInfo;

	void InitContentViewer(int content_viewer_type);
	int LoadData(const FILE_PATH_CHAR* msg_file_path, std::string& out_info);
	void RefreshHeaderView();
	MimeNode* FindRootViewNode();
	static wxString ComposeStructViewContent(const wxString& struct_info, const FILE_PATH_CHAR* msg_file_path);
	static wxString ComposeTextViewContent(const MimeNode* node);
	ContentViewer::ContentData ContentEmbeddedDataProvider(const TCHAR* id);
	int RefreshView(bool update_content);
	void InitAttachmentView();

	void btnAttachmentFile_Clicked(wxCommandEvent& event);

	//****** MailMsgViewerUI override ******
	virtual void toolSwitchContentView_OnToolClicked(wxCommandEvent& event) override;
	virtual void toolSaveContent_OnToolClicked(wxCommandEvent& event) override;
	virtual void toolOpenMessage_OnToolClicked(wxCommandEvent& event) override;
	virtual void mnuAttachmentFileSave_OnMenuSelection(wxCommandEvent& event) override;
	virtual void btnDownloadImages_OnButtonClick(wxCommandEvent& event) override;

	// ****** MaiMsgFileView override ******
	virtual int OnMailMsgFileSet();
public:
	MailMsgViewer(wxWindow* parent);
	virtual ~MailMsgViewer();
};
