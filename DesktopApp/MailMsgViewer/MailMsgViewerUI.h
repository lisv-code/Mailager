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
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/wrapsizer.h>
#include <wx/stattext.h>
#include <wx/button.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class MailMsgViewerUI
///////////////////////////////////////////////////////////////////////////////
class MailMsgViewerUI : public wxPanel
{
	private:

	protected:
		wxToolBar* tlbrMain;
		wxToolBarToolBase* toolSwitchContentView;
		wxToolBarToolBase* toolSaveContent;
		wxToolBarToolBase* toolOpenMessage;
		wxPanel* pnlHeader;
		wxTextCtrl* txtSubject;
		wxTextCtrl* txtDate;
		wxTextCtrl* txtSender;
		wxPanel* pnlAttachments;
		wxPanel* pnlExtDownload;
		wxStaticText* lblExternalImages;
		wxButton* btnDownloadImages;
		wxPanel* pnlContentViewerPlaceholder;

		// Virtual event handlers, override them in your derived class
		virtual void toolSwitchContentView_OnToolClicked( wxCommandEvent& event ) = 0;
		virtual void toolSaveContent_OnToolClicked( wxCommandEvent& event ) = 0;
		virtual void toolOpenMessage_OnToolClicked( wxCommandEvent& event ) = 0;
		virtual void btnDownloadImages_OnButtonClick( wxCommandEvent& event ) = 0;


	public:

		MailMsgViewerUI( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxTAB_TRAVERSAL, const wxString& name = wxEmptyString );

		~MailMsgViewerUI();

};

