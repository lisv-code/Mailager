///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "MailMsgEditorUI.h"

///////////////////////////////////////////////////////////////////////////

MailMsgEditorUI::MailMsgEditorUI( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxPanel( parent, id, pos, size, style, name )
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	tlbrMain = new wxToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL );
	tlbrMain->SetToolSeparation( 4 );
	toolSaveFile = tlbrMain->AddTool( wxID_ANY, wxT("save"), wxArtProvider::GetBitmap( wxASCII_STR("IcoToolSave"), wxASCII_STR(wxART_OTHER) ), wxNullBitmap, wxITEM_NORMAL, wxT("Save draft"), wxEmptyString, NULL );

	toolSendMail = tlbrMain->AddTool( wxID_ANY, wxT("open"), wxArtProvider::GetBitmap( wxASCII_STR("IcoToolExport"), wxASCII_STR(wxART_OTHER) ), wxNullBitmap, wxITEM_NORMAL, wxT("Send mail"), wxEmptyString, NULL );

	tlbrMain->Realize();

	bSizer1->Add( tlbrMain, 0, wxEXPAND, 4 );

	pnlHeader = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer1->AddGrowableCol( 1 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	lblSender = new wxStaticText( pnlHeader, wxID_ANY, wxT("From"), wxDefaultPosition, wxDefaultSize, 0 );
	lblSender->Wrap( -1 );
	fgSizer1->Add( lblSender, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 2 );

	wxArrayString chcSenderChoices;
	chcSender = new wxChoice( pnlHeader, wxID_ANY, wxDefaultPosition, wxDefaultSize, chcSenderChoices, 0 );
	chcSender->SetSelection( 0 );
	fgSizer1->Add( chcSender, 0, wxALL|wxEXPAND, 2 );

	lblRecipient = new wxStaticText( pnlHeader, wxID_ANY, wxT("To"), wxDefaultPosition, wxDefaultSize, 0 );
	lblRecipient->Wrap( -1 );
	fgSizer1->Add( lblRecipient, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 2 );

	txtRecipient = new wxTextCtrl( pnlHeader, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( txtRecipient, 0, wxALL|wxEXPAND, 2 );

	lblSubject = new wxStaticText( pnlHeader, wxID_ANY, wxT("Subject"), wxDefaultPosition, wxDefaultSize, 0 );
	lblSubject->Wrap( -1 );
	fgSizer1->Add( lblSubject, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 2 );

	txtSubject = new wxTextCtrl( pnlHeader, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( txtSubject, 0, wxALL|wxEXPAND, 2 );


	pnlHeader->SetSizer( fgSizer1 );
	pnlHeader->Layout();
	fgSizer1->Fit( pnlHeader );
	bSizer1->Add( pnlHeader, 0, wxEXPAND | wxALL, 2 );

	pnlAttachments = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxWrapSizer* szwAttachmentFiles;
	szwAttachmentFiles = new wxWrapSizer( wxHORIZONTAL, wxREMOVE_LEADING_SPACES );


	pnlAttachments->SetSizer( szwAttachmentFiles );
	pnlAttachments->Layout();
	szwAttachmentFiles->Fit( pnlAttachments );
	mnuAttachmentFile = new wxMenu();
	wxMenuItem* mnuAttachmentFileSave;
	mnuAttachmentFileSave = new wxMenuItem( mnuAttachmentFile, wxID_ANY, wxString( wxT("Save As...") ) , wxEmptyString, wxITEM_NORMAL );
	mnuAttachmentFile->Append( mnuAttachmentFileSave );

	pnlAttachments->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( MailMsgEditorUI::pnlAttachmentsOnContextMenu ), NULL, this );

	bSizer1->Add( pnlAttachments, 0, wxEXPAND | wxALL, 2 );

	pnlContentViewerPlaceholder = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxVERTICAL );

	txtContent = new wxTextCtrl( pnlContentViewerPlaceholder, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	bSizer5->Add( txtContent, 1, wxALL|wxEXPAND, 2 );


	pnlContentViewerPlaceholder->SetSizer( bSizer5 );
	pnlContentViewerPlaceholder->Layout();
	bSizer5->Fit( pnlContentViewerPlaceholder );
	bSizer1->Add( pnlContentViewerPlaceholder, 1, wxALL|wxEXPAND, 2 );


	this->SetSizer( bSizer1 );
	this->Layout();

	// Connect Events
	this->Connect( toolSaveFile->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailMsgEditorUI::toolSaveMessage_OnToolClicked ) );
	this->Connect( toolSendMail->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailMsgEditorUI::toolSendMessage_OnToolClicked ) );
	chcSender->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( MailMsgEditorUI::chcSender_OnChoice ), NULL, this );
	mnuAttachmentFile->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MailMsgEditorUI::mnuAttachmentFileSave_OnMenuSelection ), this, mnuAttachmentFileSave->GetId());
}

MailMsgEditorUI::~MailMsgEditorUI()
{
	// Disconnect Events
	this->Disconnect( toolSaveFile->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailMsgEditorUI::toolSaveMessage_OnToolClicked ) );
	this->Disconnect( toolSendMail->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailMsgEditorUI::toolSendMessage_OnToolClicked ) );
	chcSender->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( MailMsgEditorUI::chcSender_OnChoice ), NULL, this );

	delete mnuAttachmentFile;
}
