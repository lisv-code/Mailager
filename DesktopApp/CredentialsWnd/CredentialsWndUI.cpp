///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "CredentialsWndUI.h"

///////////////////////////////////////////////////////////////////////////

CredentialsWndUI::CredentialsWndUI( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	lblInfo = new wxStaticText( this, wxID_ANY, wxT("Provide credentials:"), wxDefaultPosition, wxDefaultSize, 0 );
	lblInfo->Wrap( -1 );
	bSizer1->Add( lblInfo, 0, wxALL, 5 );

	txtUser = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	bSizer1->Add( txtUser, 0, wxALL|wxEXPAND, 4 );

	txtPswd = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
	bSizer1->Add( txtPswd, 0, wxALL|wxEXPAND, 4 );

	chkSave = new wxCheckBox( this, wxID_ANY, wxT("Save credentials"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( chkSave, 0, wxALL, 5 );

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );

	btnOk = new wxButton( this, wxID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );

	btnOk->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR("IcoBtnOk"), wxASCII_STR(wxART_OTHER) ) );
	bSizer2->Add( btnOk, 0, 0, 4 );

	btnNo = new wxButton( this, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );

	btnNo->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR("IcoBtnNo"), wxASCII_STR(wxART_OTHER) ) );
	bSizer2->Add( btnNo, 0, wxLEFT, 4 );


	bSizer1->Add( bSizer2, 0, wxALIGN_RIGHT|wxALL, 4 );


	this->SetSizer( bSizer1 );
	this->Layout();

	this->Centre( wxBOTH );
}

CredentialsWndUI::~CredentialsWndUI()
{
}
