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
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/gdicmn.h>
#include <wx/toolbar.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class MailAccCfgUI
///////////////////////////////////////////////////////////////////////////////
class MailAccCfgUI : public wxDialog
{
	private:

	protected:
		wxToolBar* tlbrAccEdit;
		wxToolBarToolBase* toolAccCreate;
		wxToolBarToolBase* toolAccDelete;
		wxChoice* chcAccount;
		wxStaticText* lblAccName;
		wxTextCtrl* txtAccName;
		wxStaticText* lblEmailAddr;
		wxTextCtrl* txtEmailAddr;
		wxStaticText* lblIncoming;
		wxStaticText* lblOutgoing;
		wxStaticText* lblProto;
		wxChoice* chcIncProto;
		wxChoice* chcOutProto;
		wxStaticText* lblSsl;
		wxCheckBox* chkIncSsl;
		wxCheckBox* chkOutSsl;
		wxStaticText* lblServer;
		wxTextCtrl* txtIncServer;
		wxTextCtrl* txtOutServer;
		wxStaticText* lblPort;
		wxTextCtrl* txtIncPort;
		wxTextCtrl* txtOutPort;
		wxStaticText* lblUser;
		wxTextCtrl* txtIncUser;
		wxTextCtrl* txtOutUser;
		wxStaticText* lblAuth;
		wxChoice* chcIncAuth;
		wxChoice* chcOutAuth;
		wxButton* btnOk;
		wxButton* btnNo;

		// Virtual event handlers, override them in your derived class
		virtual void MailAccCfgUI_OnClose( wxCloseEvent& event ) = 0;
		virtual void MailAccCfgUI_OnInitDialog( wxInitDialogEvent& event ) = 0;
		virtual void toolAccCreate_OnToolClicked( wxCommandEvent& event ) = 0;
		virtual void toolAccDelete_OnToolClicked( wxCommandEvent& event ) = 0;
		virtual void chcAccount_OnChoice( wxCommandEvent& event ) = 0;
		virtual void btnOk_OnButtonClick( wxCommandEvent& event ) = 0;
		virtual void btnNo_OnButtonClick( wxCommandEvent& event ) = 0;


	public:

		MailAccCfgUI( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Mail Accounts"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxRESIZE_BORDER );

		~MailAccCfgUI();

};

