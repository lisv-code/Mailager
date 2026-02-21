///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/toolbar.h>
#include <wx/listbox.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class OAuth2CfgUI
///////////////////////////////////////////////////////////////////////////////
class OAuth2CfgUI : public wxDialog
{
	private:

	protected:
		wxStaticText* lblVerifReq;
		wxCheckBox* chkVerifReq;
		wxToolBar* tlbrProvEdit;
		wxToolBarToolBase* toolProvCreate;
		wxToolBarToolBase* toolProvDelete;
		wxListBox* lbxProviders;
		wxStaticText* lblProvName;
		wxTextCtrl* txtProvName;
		wxCheckBox* chkProvStatus;
		wxStaticText* lblAuthEndpoint;
		wxTextCtrl* txtAuthEndpoint;
		wxStaticText* lblTokenEndpoint;
		wxTextCtrl* txtTokenEndpoint;
		wxStaticText* lblScope;
		wxTextCtrl* txtScope;
		wxStaticText* lblClientId;
		wxTextCtrl* txtClientId;
		wxStaticText* lblClientSecret;
		wxTextCtrl* txtClientSecret;
		wxStaticText* lblComment;
		wxTextCtrl* txtComment;
		wxButton* btnOk;
		wxButton* btnNo;

		// Virtual event handlers, override them in your derived class
		virtual void OAuth2CfgUI_OnClose( wxCloseEvent& event ) = 0;
		virtual void OAuth2CfgUI_OnInitDialog( wxInitDialogEvent& event ) = 0;
		virtual void toolProvCreate_OnToolClicked( wxCommandEvent& event ) = 0;
		virtual void toolProvDelete_OnToolClicked( wxCommandEvent& event ) = 0;
		virtual void lbxProviders_OnListBox( wxCommandEvent& event ) = 0;
		virtual void btnOk_OnButtonClick( wxCommandEvent& event ) = 0;
		virtual void btnNo_OnButtonClick( wxCommandEvent& event ) = 0;


	public:

		OAuth2CfgUI( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("OAuth2 configuration"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );

		~OAuth2CfgUI();

};

