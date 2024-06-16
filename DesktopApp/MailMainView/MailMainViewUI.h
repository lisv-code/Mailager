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
#include <wx/dataview.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/combobox.h>
#include <wx/menu.h>
#include <wx/splitter.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class MailMainViewUI
///////////////////////////////////////////////////////////////////////////////
class MailMainViewUI : public wxPanel
{
	private:

	protected:
		wxSplitterWindow* wndMasterSplitter;
		wxPanel* pnlMasterView;
		wxToolBar* tlbrMaster;
		wxToolBarToolBase* toolConfigMasterView;
		wxToolBarToolBase* toolStartSyncMail;
		wxToolBarToolBase* toolStopSyncMail;
		wxToolBarToolBase* toolCreateMailMsg;
		wxDataViewCtrl* dvAccFolders;
		wxDataViewColumn* dvcAccFoldersCol0;
		wxDataViewColumn* dvcAccFoldersCol1;
		wxPanel* pnlDetailView;
		wxSplitterWindow* wndDetailSplitter;
		wxPanel* pnlMailMsgList;
		wxToolBar* tlbrDetail;
		wxToolBarToolBase* toolMailMsgFilterSwitch;
		wxComboBox* cmbMailMsgFilterValue;
		wxToolBarToolBase* toolMailMsgFilterApply;
		wxToolBarToolBase* toolMailMsgLayout;
		wxDataViewCtrl* dvMailMsgList;
		wxDataViewColumn* dvcDetailCol0;
		wxMenu* mnuMailMsgItem;
		wxPanel* pnlMailMsgView;

		// Virtual event handlers, override them in your derived class
		virtual void toolConfigMasterView_OnToolClicked( wxCommandEvent& event ) = 0;
		virtual void toolStartSyncMail_OnToolClicked( wxCommandEvent& event ) = 0;
		virtual void toolStopSyncMail_OnToolClicked( wxCommandEvent& event ) = 0;
		virtual void toolCreateMailMsg_OnToolClicked( wxCommandEvent& event ) = 0;
		virtual void dvAccFolders_OnDataViewCtrlSelectionChanged( wxDataViewEvent& event ) = 0;
		virtual void toolMailMsgFilterSwitch_OnToolClicked( wxCommandEvent& event ) = 0;
		virtual void cmbMailMsgFilterValue_OnKeyDown( wxKeyEvent& event ) = 0;
		virtual void cmbMailMsgFilterValue_OnText( wxCommandEvent& event ) = 0;
		virtual void cmbMailMsgFilterValue_OnTextEnter( wxCommandEvent& event ) = 0;
		virtual void toolMailMsgFilterApply_OnToolClicked( wxCommandEvent& event ) = 0;
		virtual void toolMailMsgLayout_OnToolClicked( wxCommandEvent& event ) = 0;
		virtual void dvMailMsgList_OnDataViewCtrlItemActivated( wxDataViewEvent& event ) = 0;
		virtual void dvMailMsgList_OnDataViewCtrlItemContextMenu( wxDataViewEvent& event ) = 0;
		virtual void dvMailMsgList_OnDataViewCtrlSelectionChanged( wxDataViewEvent& event ) = 0;
		virtual void mnuMailMsgItemOpen_OnMenuSelection( wxCommandEvent& event ) = 0;
		virtual void mnuMailMsgItemMarkAsRead_OnMenuSelection( wxCommandEvent& event ) = 0;
		virtual void mnuMailMsgItemMarkUnread_OnMenuSelection( wxCommandEvent& event ) = 0;
		virtual void mnuMailMsgItemDelete_OnMenuSelection( wxCommandEvent& event ) = 0;


	public:

		MailMainViewUI( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 620,460 ), long style = wxTAB_TRAVERSAL, const wxString& name = wxEmptyString );

		~MailMainViewUI();

		void wndMasterSplitterOnIdle( wxIdleEvent& )
		{
			wndMasterSplitter->SetSashPosition( 220 );
			wndMasterSplitter->Disconnect( wxEVT_IDLE, wxIdleEventHandler( MailMainViewUI::wndMasterSplitterOnIdle ), NULL, this );
		}

		void wndDetailSplitterOnIdle( wxIdleEvent& )
		{
			wndDetailSplitter->SetSashPosition( 360 );
			wndDetailSplitter->Disconnect( wxEVT_IDLE, wxIdleEventHandler( MailMainViewUI::wndDetailSplitterOnIdle ), NULL, this );
		}

		void pnlMailMsgListOnContextMenu( wxMouseEvent &event )
		{
			pnlMailMsgList->PopupMenu( mnuMailMsgItem, event.GetPosition() );
		}

};

