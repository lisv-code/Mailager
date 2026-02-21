///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "OAuth2CfgUI.h"

///////////////////////////////////////////////////////////////////////////

OAuth2CfgUI::OAuth2CfgUI( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );

	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer1->AddGrowableCol( 1 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	lblVerifReq = new wxStaticText( this, wxID_ANY, wxT("Verification required"), wxDefaultPosition, wxDefaultSize, 0 );
	lblVerifReq->Wrap( -1 );
	lblVerifReq->Enable( false );

	fgSizer1->Add( lblVerifReq, 0, wxALL, 5 );

	chkVerifReq = new wxCheckBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	chkVerifReq->Enable( false );

	fgSizer1->Add( chkVerifReq, 0, wxALL, 5 );


	bSizer2->Add( fgSizer1, 0, wxALL|wxEXPAND, 2 );

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxVERTICAL );

	tlbrProvEdit = new wxToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL );
	toolProvCreate = tlbrProvEdit->AddTool( wxID_ANY, wxT("create"), wxArtProvider::GetBitmap( wxASCII_STR("IcoToolCreate"), wxASCII_STR(wxART_OTHER) ), wxNullBitmap, wxITEM_NORMAL, wxT("Add new provider"), wxEmptyString, NULL );

	toolProvDelete = tlbrProvEdit->AddTool( wxID_ANY, wxT("tool"), wxArtProvider::GetBitmap( wxASCII_STR("IcoToolDelete"), wxASCII_STR(wxART_OTHER) ), wxNullBitmap, wxITEM_NORMAL, wxT("Remove provider"), wxEmptyString, NULL );

	tlbrProvEdit->Realize();

	bSizer5->Add( tlbrProvEdit, 0, wxEXPAND, 5 );

	lbxProviders = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, 0 );
	lbxProviders->SetMaxSize( wxSize( 220,-1 ) );

	bSizer5->Add( lbxProviders, 1, wxALL|wxEXPAND, 2 );


	bSizer4->Add( bSizer5, 0, wxALL|wxEXPAND, 2 );

	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxHORIZONTAL );

	lblProvName = new wxStaticText( this, wxID_ANY, wxT("Name"), wxDefaultPosition, wxDefaultSize, 0 );
	lblProvName->Wrap( -1 );
	bSizer7->Add( lblProvName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );

	txtProvName = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer7->Add( txtProvName, 0, wxALL, 2 );


	bSizer7->Add( 0, 0, 1, wxALL|wxEXPAND, 8 );

	chkProvStatus = new wxCheckBox( this, wxID_ANY, wxT("enabled"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer7->Add( chkProvStatus, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2 );


	bSizer6->Add( bSizer7, 0, wxALL|wxEXPAND, 6 );

	lblAuthEndpoint = new wxStaticText( this, wxID_ANY, wxT("Authorization endpoint"), wxDefaultPosition, wxDefaultSize, 0 );
	lblAuthEndpoint->Wrap( -1 );
	bSizer6->Add( lblAuthEndpoint, 0, wxLEFT|wxRIGHT|wxTOP, 2 );

	txtAuthEndpoint = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer6->Add( txtAuthEndpoint, 0, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 2 );

	lblTokenEndpoint = new wxStaticText( this, wxID_ANY, wxT("Token endpoint"), wxDefaultPosition, wxDefaultSize, 0 );
	lblTokenEndpoint->Wrap( -1 );
	bSizer6->Add( lblTokenEndpoint, 0, wxLEFT|wxRIGHT|wxTOP, 2 );

	txtTokenEndpoint = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer6->Add( txtTokenEndpoint, 0, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 2 );

	lblScope = new wxStaticText( this, wxID_ANY, wxT("Scope"), wxDefaultPosition, wxDefaultSize, 0 );
	lblScope->Wrap( -1 );
	bSizer6->Add( lblScope, 0, wxLEFT|wxRIGHT|wxTOP, 2 );

	txtScope = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer6->Add( txtScope, 0, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 2 );

	lblClientId = new wxStaticText( this, wxID_ANY, wxT("Client ID"), wxDefaultPosition, wxDefaultSize, 0 );
	lblClientId->Wrap( -1 );
	bSizer6->Add( lblClientId, 0, wxLEFT|wxRIGHT|wxTOP, 2 );

	txtClientId = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer6->Add( txtClientId, 0, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 2 );

	lblClientSecret = new wxStaticText( this, wxID_ANY, wxT("Client secret"), wxDefaultPosition, wxDefaultSize, 0 );
	lblClientSecret->Wrap( -1 );
	bSizer6->Add( lblClientSecret, 0, wxLEFT|wxRIGHT|wxTOP, 2 );

	txtClientSecret = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer6->Add( txtClientSecret, 0, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 2 );

	lblComment = new wxStaticText( this, wxID_ANY, wxT("Comment"), wxDefaultPosition, wxDefaultSize, 0 );
	lblComment->Wrap( -1 );
	bSizer6->Add( lblComment, 0, wxLEFT|wxRIGHT|wxTOP, 2 );

	txtComment = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_AUTO_URL|wxTE_BESTWRAP|wxTE_CHARWRAP|wxTE_MULTILINE|wxTE_WORDWRAP );
	bSizer6->Add( txtComment, 1, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 2 );


	bSizer4->Add( bSizer6, 1, wxALL|wxEXPAND, 2 );


	bSizer2->Add( bSizer4, 1, wxEXPAND, 2 );


	bSizer1->Add( bSizer2, 1, wxALL|wxEXPAND, 2 );

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );

	btnOk = new wxButton( this, wxID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );

	btnOk->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR("IcoBtnOk"), wxASCII_STR(wxART_OTHER) ) );
	bSizer3->Add( btnOk, 0, wxALL, 2 );

	btnNo = new wxButton( this, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );

	btnNo->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR("IcoBtnNo"), wxASCII_STR(wxART_OTHER) ) );
	bSizer3->Add( btnNo, 0, wxALL, 2 );


	bSizer1->Add( bSizer3, 0, wxALIGN_RIGHT|wxALL, 4 );


	this->SetSizer( bSizer1 );
	this->Layout();
	bSizer1->Fit( this );

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( OAuth2CfgUI::OAuth2CfgUI_OnClose ) );
	this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( OAuth2CfgUI::OAuth2CfgUI_OnInitDialog ) );
	this->Connect( toolProvCreate->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( OAuth2CfgUI::toolProvCreate_OnToolClicked ) );
	this->Connect( toolProvDelete->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( OAuth2CfgUI::toolProvDelete_OnToolClicked ) );
	lbxProviders->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( OAuth2CfgUI::lbxProviders_OnListBox ), NULL, this );
	btnOk->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( OAuth2CfgUI::btnOk_OnButtonClick ), NULL, this );
	btnNo->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( OAuth2CfgUI::btnNo_OnButtonClick ), NULL, this );
}

OAuth2CfgUI::~OAuth2CfgUI()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( OAuth2CfgUI::OAuth2CfgUI_OnClose ) );
	this->Disconnect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( OAuth2CfgUI::OAuth2CfgUI_OnInitDialog ) );
	this->Disconnect( toolProvCreate->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( OAuth2CfgUI::toolProvCreate_OnToolClicked ) );
	this->Disconnect( toolProvDelete->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( OAuth2CfgUI::toolProvDelete_OnToolClicked ) );
	lbxProviders->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( OAuth2CfgUI::lbxProviders_OnListBox ), NULL, this );
	btnOk->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( OAuth2CfgUI::btnOk_OnButtonClick ), NULL, this );
	btnNo->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( OAuth2CfgUI::btnNo_OnButtonClick ), NULL, this );

}
