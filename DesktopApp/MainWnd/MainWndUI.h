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
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/toolbar.h>
#include <wx/aui/auibook.h>
#include <wx/sizer.h>
#include <wx/statusbr.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class MainWndUI
///////////////////////////////////////////////////////////////////////////////
class MainWndUI : public wxFrame
{
	private:

	protected:
		wxMenuBar* mnubarMain;
		wxMenu* mnuFile;
		wxMenu* mnuEdit;
		wxMenu* mnuView;
		wxMenu* mnuTools;
		wxMenu* mnuHelp;
		wxToolBar* tbarMain;
		wxToolBarToolBase* toolEditAction1;
		wxToolBarToolBase* toolEditAction2;
		wxAuiNotebook* tabCtrlMain;
		wxStatusBar* sbarMain;

		// Virtual event handlers, override them in your derived class
		virtual void mnuFileExit_OnMenuSelection( wxCommandEvent& event ) = 0;
		virtual void mnuEditNewMailMessageOnMenuSelection( wxCommandEvent& event ) = 0;
		virtual void mnuViewToolbar_OnMenuSelection( wxCommandEvent& event ) = 0;
		virtual void mnuViewStatusBar_OnMenuSelection( wxCommandEvent& event ) = 0;
		virtual void mnuViewLog_OnMenuSelection( wxCommandEvent& event ) = 0;
		virtual void mnuToolsAccountsConfig_OnMenuSelection( wxCommandEvent& event ) = 0;
		virtual void mnuHelpAbout_OnMenuSelection( wxCommandEvent& event ) = 0;
		virtual void toolEditAction1_OnToolClicked( wxCommandEvent& event ) = 0;
		virtual void toolEditAction2_OnToolClicked( wxCommandEvent& event ) = 0;
		virtual void tabCtrlMain_OnAuiNotebookPageChanged( wxAuiNotebookEvent& event ) = 0;


	public:

		MainWndUI( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Mailager"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 760,520 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~MainWndUI();

};

