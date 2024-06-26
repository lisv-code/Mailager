﻿///////////////////////////////////////////////////////////////////////////
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
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/sizer.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class CredentialsWndUI
///////////////////////////////////////////////////////////////////////////////
class CredentialsWndUI : public wxDialog
{
	private:

	protected:
		wxStaticText* lblInfo;
		wxTextCtrl* txtUser;
		wxTextCtrl* txtPswd;
		wxCheckBox* chkSave;
		wxButton* btnOk;
		wxButton* btnNo;

	public:

		CredentialsWndUI( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Credentials"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 310,190 ), long style = wxCLOSE_BOX|wxDEFAULT_DIALOG_STYLE );

		~CredentialsWndUI();

};

