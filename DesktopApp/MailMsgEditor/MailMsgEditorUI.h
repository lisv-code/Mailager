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
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/wrapsizer.h>
#include <wx/menu.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class MailMsgEditorUI
///////////////////////////////////////////////////////////////////////////////
class MailMsgEditorUI : public wxPanel
{
	private:

	protected:
		wxToolBar* tlbrMain;
		wxToolBarToolBase* toolSaveMessage;
		wxToolBarToolBase* toolSendMessage;
		wxToolBarToolBase* toolAddAttachment;
		wxPanel* pnlHeader;
		wxStaticText* lblSender;
		wxChoice* chcSender;
		wxStaticText* lblRecipient;
		wxTextCtrl* txtRecipient;
		wxStaticText* lblSubject;
		wxTextCtrl* txtSubject;
		wxPanel* pnlAttachments;
		wxMenu* mnuAttachments;
		wxPanel* pnlContentViewerPlaceholder;
		wxTextCtrl* txtContent;

		// Virtual event handlers, override them in your derived class
		virtual void toolSaveMessage_OnToolClicked( wxCommandEvent& event ) = 0;
		virtual void toolSendMessage_OnToolClicked( wxCommandEvent& event ) = 0;
		virtual void toolAddAttachment_OnToolClicked( wxCommandEvent& event ) = 0;
		virtual void chcSender_OnChoice( wxCommandEvent& event ) = 0;
		virtual void mnuAttachmentsAdd_OnMenuSelection( wxCommandEvent& event ) = 0;


	public:

		MailMsgEditorUI( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxTAB_TRAVERSAL, const wxString& name = wxEmptyString );

		~MailMsgEditorUI();

		void pnlAttachmentsOnContextMenu( wxMouseEvent &event )
		{
			pnlAttachments->PopupMenu( mnuAttachments, event.GetPosition() );
		}

};

