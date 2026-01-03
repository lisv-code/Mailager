///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "MailMsgViewerUI.h"

///////////////////////////////////////////////////////////////////////////

MailMsgViewerUI::MailMsgViewerUI( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxPanel( parent, id, pos, size, style, name )
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	tlbrMain = new wxToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL );
	tlbrMain->SetToolSeparation( 4 );
	toolSwitchContentView = tlbrMain->AddTool( wxID_ANY, wxT("view"), wxArtProvider::GetBitmap( wxASCII_STR("IcoToolView"), wxASCII_STR(wxART_OTHER) ), wxNullBitmap, wxITEM_NORMAL, wxT("Switch content view type"), wxEmptyString, NULL );

	tlbrMain->AddSeparator();

	toolSaveContent = tlbrMain->AddTool( wxID_ANY, wxT("save"), wxArtProvider::GetBitmap( wxASCII_STR("IcoToolSave"), wxASCII_STR(wxART_OTHER) ), wxNullBitmap, wxITEM_NORMAL, wxT("Save message content"), wxEmptyString, NULL );

	toolOpenMessage = tlbrMain->AddTool( wxID_ANY, wxT("open"), wxArtProvider::GetBitmap( wxASCII_STR("IcoToolExport"), wxASCII_STR(wxART_OTHER) ), wxNullBitmap, wxITEM_NORMAL, wxT("Open message externally"), wxEmptyString, NULL );

	tlbrMain->Realize();

	bSizer1->Add( tlbrMain, 0, wxEXPAND, 4 );

	pnlHeader = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );

	txtSubject = new wxTextCtrl( pnlHeader, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	bSizer2->Add( txtSubject, 0, wxALL|wxEXPAND, 2 );

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );

	txtDate = new wxTextCtrl( pnlHeader, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	bSizer3->Add( txtDate, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxFIXED_MINSIZE, 2 );

	txtSender = new wxTextCtrl( pnlHeader, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	bSizer3->Add( txtSender, 1, wxALL|wxEXPAND, 2 );


	bSizer2->Add( bSizer3, 1, wxEXPAND, 0 );


	pnlHeader->SetSizer( bSizer2 );
	pnlHeader->Layout();
	bSizer2->Fit( pnlHeader );
	bSizer1->Add( pnlHeader, 0, wxEXPAND | wxALL, 2 );

	pnlAttachments = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	pnlAttachments->Hide();

	wxWrapSizer* szwAttachmentFiles;
	szwAttachmentFiles = new wxWrapSizer( wxHORIZONTAL, wxREMOVE_LEADING_SPACES );


	pnlAttachments->SetSizer( szwAttachmentFiles );
	pnlAttachments->Layout();
	szwAttachmentFiles->Fit( pnlAttachments );
	bSizer1->Add( pnlAttachments, 0, wxEXPAND | wxALL, 2 );

	pnlExtDownload = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	pnlExtDownload->Hide();

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxHORIZONTAL );

	lblExternalImages = new wxStaticText( pnlExtDownload, wxID_ANY, wxT("This message includes images from external resources."), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	lblExternalImages->Wrap( -1 );
	bSizer4->Add( lblExternalImages, 1, wxALIGN_CENTER_VERTICAL|wxALL, 2 );

	btnDownloadImages = new wxButton( pnlExtDownload, wxID_ANY, wxT("Download Images"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer4->Add( btnDownloadImages, 0, wxALL, 2 );


	pnlExtDownload->SetSizer( bSizer4 );
	pnlExtDownload->Layout();
	bSizer4->Fit( pnlExtDownload );
	bSizer1->Add( pnlExtDownload, 0, wxEXPAND | wxALL, 2 );

	pnlContentViewerPlaceholder = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	bSizer1->Add( pnlContentViewerPlaceholder, 1, wxALL|wxEXPAND, 2 );


	this->SetSizer( bSizer1 );
	this->Layout();

	// Connect Events
	this->Connect( toolSwitchContentView->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailMsgViewerUI::toolSwitchContentView_OnToolClicked ) );
	this->Connect( toolSaveContent->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailMsgViewerUI::toolSaveContent_OnToolClicked ) );
	this->Connect( toolOpenMessage->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailMsgViewerUI::toolOpenMessage_OnToolClicked ) );
	btnDownloadImages->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MailMsgViewerUI::btnDownloadImages_OnButtonClick ), NULL, this );
}

MailMsgViewerUI::~MailMsgViewerUI()
{
	// Disconnect Events
	this->Disconnect( toolSwitchContentView->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailMsgViewerUI::toolSwitchContentView_OnToolClicked ) );
	this->Disconnect( toolSaveContent->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailMsgViewerUI::toolSaveContent_OnToolClicked ) );
	this->Disconnect( toolOpenMessage->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailMsgViewerUI::toolOpenMessage_OnToolClicked ) );
	btnDownloadImages->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MailMsgViewerUI::btnDownloadImages_OnButtonClick ), NULL, this );

}
