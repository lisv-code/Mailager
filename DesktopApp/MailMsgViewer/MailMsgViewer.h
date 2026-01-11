#pragma once
#include <string>
#include "MailMsgViewerUI.h"
#include "../MailMsgFileView.h"
#include "../../CoreMailLib/MimeNode.h"
#include "../ContentViewer/ContentViewer.h"
#include "../MailMsgCtrlAttachments/MailMsgCtrlAttachments.h"
#include "MimeNodeViewInfo.h"

namespace MailMsgViewer_Def
{
	extern const TCHAR* WndTitle;
}

class MailMsgViewer : public MailMsgViewerUI, public MailMsgFileView
{
	ContentViewer* contentViewer;
	MailMsgCtrlAttachments attachmentsCtrl;
	MimeNode msgNode;
	MimeNodeViewInfo::NodeStructInfoContainer nodeStruct;

	bool isViewMsgStruct = false;
	wxString structInfo;

	void InitContentViewer(int content_viewer_type);
	int LoadData(std::string& out_info);
	void RefreshHeaderView();
	MimeNode* FindRootViewNode();
	static wxString ComposeStructViewContent(const wxString& struct_info, const FILE_PATH_CHAR* msg_file_path);
	static wxString ComposeTextViewContent(const MimeNode* node);
	ContentViewer::ContentData ContentEmbeddedDataProvider(const TCHAR* id);
	int RefreshView(bool update_content);

	//****** MailMsgViewerUI override ******
	virtual void toolSwitchContentView_OnToolClicked(wxCommandEvent& event) override;
	virtual void toolSaveContent_OnToolClicked(wxCommandEvent& event) override;
	virtual void toolOpenMessage_OnToolClicked(wxCommandEvent& event) override;
	virtual void btnDownloadImages_OnButtonClick(wxCommandEvent& event) override;

	// ****** MaiMsgFileView override ******
	virtual int OnMailMsgFileChanged(MailMsgFile* prev_value) override;
public:
	MailMsgViewer(wxWindow* parent);
	virtual ~MailMsgViewer();
};
