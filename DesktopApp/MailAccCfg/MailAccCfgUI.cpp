///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "MailAccCfgUI.h"

///////////////////////////////////////////////////////////////////////////

MailAccCfgUI::MailAccCfgUI( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxHORIZONTAL );

	tlbrAccEdit = new wxToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL|wxBORDER_SIMPLE );
	tlbrAccEdit->SetToolSeparation( 4 );
	toolAccCreate = tlbrAccEdit->AddTool( wxID_ANY, wxT("create"), wxArtProvider::GetBitmap( wxASCII_STR("IcoToolCreate"), wxASCII_STR(wxART_OTHER) ), wxNullBitmap, wxITEM_NORMAL, wxT("Create new account"), wxEmptyString, NULL );

	toolAccDelete = tlbrAccEdit->AddTool( wxID_ANY, wxT("delete"), wxArtProvider::GetBitmap( wxASCII_STR("IcoToolDelete"), wxASCII_STR(wxART_OTHER) ), wxNullBitmap, wxITEM_NORMAL, wxT("Delete account"), wxEmptyString, NULL );

	tlbrAccEdit->Realize();

	bSizer5->Add( tlbrAccEdit, 0, wxALL|wxEXPAND, 4 );

	wxArrayString chcAccountChoices;
	chcAccount = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, chcAccountChoices, 0 );
	chcAccount->SetSelection( 0 );
	bSizer5->Add( chcAccount, 1, wxALL|wxEXPAND, 4 );


	bSizer3->Add( bSizer5, 0, wxALL|wxEXPAND, 2 );

	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer3->AddGrowableCol( 1 );
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	lblAccName = new wxStaticText( this, wxID_ANY, wxT("Account name"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAccName->Wrap( -1 );
	fgSizer3->Add( lblAccName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );

	txtAccName = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer3->Add( txtAccName, 0, wxALL|wxEXPAND, 2 );

	lblEmailAddr = new wxStaticText( this, wxID_ANY, wxT("E-mail address"), wxDefaultPosition, wxDefaultSize, 0 );
	lblEmailAddr->Wrap( -1 );
	fgSizer3->Add( lblEmailAddr, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );

	txtEmailAddr = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer3->Add( txtEmailAddr, 0, wxALL|wxEXPAND, 2 );


	bSizer3->Add( fgSizer3, 0, wxALL|wxEXPAND, 2 );

	wxFlexGridSizer* fgSizer4;
	fgSizer4 = new wxFlexGridSizer( 0, 3, 2, 8 );
	fgSizer4->AddGrowableCol( 1 );
	fgSizer4->AddGrowableCol( 2 );
	fgSizer4->SetFlexibleDirection( wxBOTH );
	fgSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );


	fgSizer4->Add( 0, 0, 1, wxEXPAND, 5 );

	lblIncoming = new wxStaticText( this, wxID_ANY, wxT("Incoming"), wxDefaultPosition, wxDefaultSize, 0 );
	lblIncoming->Wrap( -1 );
	fgSizer4->Add( lblIncoming, 0, wxALIGN_CENTER|wxALL, 4 );

	lblOutgoing = new wxStaticText( this, wxID_ANY, wxT("Outgoing"), wxDefaultPosition, wxDefaultSize, 0 );
	lblOutgoing->Wrap( -1 );
	fgSizer4->Add( lblOutgoing, 0, wxALIGN_CENTER|wxALL, 4 );

	lblProto = new wxStaticText( this, wxID_ANY, wxT("Protocol"), wxDefaultPosition, wxDefaultSize, 0 );
	lblProto->Wrap( -1 );
	fgSizer4->Add( lblProto, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );

	wxString chcIncProtoChoices[] = { wxT("POP3 (Post Office Protocol)") };
	int chcIncProtoNChoices = sizeof( chcIncProtoChoices ) / sizeof( wxString );
	chcIncProto = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, chcIncProtoNChoices, chcIncProtoChoices, 0 );
	chcIncProto->SetSelection( 0 );
	fgSizer4->Add( chcIncProto, 0, wxALL|wxEXPAND, 2 );

	wxString chcOutProtoChoices[] = { wxT("SMTP (Simple Mail Transfer Protocol)") };
	int chcOutProtoNChoices = sizeof( chcOutProtoChoices ) / sizeof( wxString );
	chcOutProto = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, chcOutProtoNChoices, chcOutProtoChoices, 0 );
	chcOutProto->SetSelection( 0 );
	fgSizer4->Add( chcOutProto, 0, wxALL|wxEXPAND, 2 );

	lblSsl = new wxStaticText( this, wxID_ANY, wxT("SSL"), wxDefaultPosition, wxDefaultSize, 0 );
	lblSsl->Wrap( -1 );
	fgSizer4->Add( lblSsl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );

	chkIncSsl = new wxCheckBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer4->Add( chkIncSsl, 0, wxALL, 2 );

	chkOutSsl = new wxCheckBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer4->Add( chkOutSsl, 0, wxALL, 2 );

	lblServer = new wxStaticText( this, wxID_ANY, wxT("Server"), wxDefaultPosition, wxDefaultSize, 0 );
	lblServer->Wrap( -1 );
	fgSizer4->Add( lblServer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );

	txtIncServer = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer4->Add( txtIncServer, 0, wxALL|wxEXPAND, 2 );

	txtOutServer = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer4->Add( txtOutServer, 0, wxALL|wxEXPAND, 2 );

	lblPort = new wxStaticText( this, wxID_ANY, wxT("Port"), wxDefaultPosition, wxDefaultSize, 0 );
	lblPort->Wrap( -1 );
	fgSizer4->Add( lblPort, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );

	txtIncPort = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer4->Add( txtIncPort, 0, wxALL, 2 );

	txtOutPort = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer4->Add( txtOutPort, 0, wxALL, 2 );

	lblUser = new wxStaticText( this, wxID_ANY, wxT("User name"), wxDefaultPosition, wxDefaultSize, 0 );
	lblUser->Wrap( -1 );
	fgSizer4->Add( lblUser, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );

	txtIncUser = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer4->Add( txtIncUser, 0, wxALL|wxEXPAND, 2 );

	txtOutUser = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer4->Add( txtOutUser, 0, wxALL|wxEXPAND, 2 );

	lblAuth = new wxStaticText( this, wxID_ANY, wxT("Authentication"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAuth->Wrap( -1 );
	fgSizer4->Add( lblAuth, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );

	wxString chcIncAuthChoices[] = { wxT("User password"), wxT("OAuth 2 (Google)") };
	int chcIncAuthNChoices = sizeof( chcIncAuthChoices ) / sizeof( wxString );
	chcIncAuth = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, chcIncAuthNChoices, chcIncAuthChoices, 0 );
	chcIncAuth->SetSelection( 0 );
	fgSizer4->Add( chcIncAuth, 0, wxALL|wxEXPAND, 2 );

	wxString chcOutAuthChoices[] = { wxT("User password"), wxT("OAuth 2 (Google)") };
	int chcOutAuthNChoices = sizeof( chcOutAuthChoices ) / sizeof( wxString );
	chcOutAuth = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, chcOutAuthNChoices, chcOutAuthChoices, 0 );
	chcOutAuth->SetSelection( 0 );
	fgSizer4->Add( chcOutAuth, 0, wxALL|wxEXPAND, 2 );


	bSizer3->Add( fgSizer4, 0, wxALL|wxEXPAND, 4 );


	bSizer4->Add( bSizer3, 0, wxALL|wxEXPAND, 2 );


	bSizer1->Add( bSizer4, 1, wxALL|wxEXPAND, 2 );

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );

	btnOk = new wxButton( this, wxID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );

	btnOk->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR("IcoBtnOk"), wxASCII_STR(wxART_OTHER) ) );
	bSizer2->Add( btnOk, 0, 0, 4 );

	btnNo = new wxButton( this, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );

	btnNo->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR("IcoBtnNo"), wxASCII_STR(wxART_OTHER) ) );
	bSizer2->Add( btnNo, 0, wxLEFT, 4 );


	bSizer1->Add( bSizer2, 0, wxALIGN_RIGHT|wxALL, 2 );


	this->SetSizer( bSizer1 );
	this->Layout();
	bSizer1->Fit( this );

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( MailAccCfgUI::MailAccCfgUI_OnClose ) );
	this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( MailAccCfgUI::MailAccCfgUI_OnInitDialog ) );
	this->Connect( toolAccCreate->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailAccCfgUI::toolAccCreate_OnToolClicked ) );
	this->Connect( toolAccDelete->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailAccCfgUI::toolAccDelete_OnToolClicked ) );
	chcAccount->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( MailAccCfgUI::chcAccount_OnChoice ), NULL, this );
	btnOk->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MailAccCfgUI::btnOk_OnButtonClick ), NULL, this );
	btnNo->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MailAccCfgUI::btnNo_OnButtonClick ), NULL, this );
}

MailAccCfgUI::~MailAccCfgUI()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( MailAccCfgUI::MailAccCfgUI_OnClose ) );
	this->Disconnect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( MailAccCfgUI::MailAccCfgUI_OnInitDialog ) );
	this->Disconnect( toolAccCreate->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailAccCfgUI::toolAccCreate_OnToolClicked ) );
	this->Disconnect( toolAccDelete->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailAccCfgUI::toolAccDelete_OnToolClicked ) );
	chcAccount->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( MailAccCfgUI::chcAccount_OnChoice ), NULL, this );
	btnOk->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MailAccCfgUI::btnOk_OnButtonClick ), NULL, this );
	btnNo->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( MailAccCfgUI::btnNo_OnButtonClick ), NULL, this );

}
