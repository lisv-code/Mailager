///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "MainWndUI.h"

///////////////////////////////////////////////////////////////////////////

MainWndUI::MainWndUI( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );

	mnubarMain = new wxMenuBar( 0 );
	mnuFile = new wxMenu();
	wxMenuItem* mnuFileExit;
	mnuFileExit = new wxMenuItem( mnuFile, wxID_ANY, wxString( wxT("Exit") ) , wxEmptyString, wxITEM_NORMAL );
	mnuFile->Append( mnuFileExit );

	mnubarMain->Append( mnuFile, wxT("File") );

	mnuEdit = new wxMenu();
	wxMenuItem* mnuEditNewMailMessage;
	mnuEditNewMailMessage = new wxMenuItem( mnuEdit, wxID_ANY, wxString( wxT("New Mail Message") ) , wxEmptyString, wxITEM_NORMAL );
	mnuEdit->Append( mnuEditNewMailMessage );

	mnubarMain->Append( mnuEdit, wxT("Edit") );

	mnuView = new wxMenu();
	wxMenuItem* mnuViewToolbar;
	mnuViewToolbar = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Toolbar") ) , wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewToolbar );

	wxMenuItem* mnuViewStatusBar;
	mnuViewStatusBar = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Status bar") ) , wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewStatusBar );

	wxMenuItem* mnuViewLog;
	mnuViewLog = new wxMenuItem( mnuView, wxID_ANY, wxString( wxT("Log") ) , wxEmptyString, wxITEM_NORMAL );
	mnuView->Append( mnuViewLog );

	mnubarMain->Append( mnuView, wxT("View") );

	mnuTools = new wxMenu();
	wxMenuItem* mnuToolsAccountsConfig;
	mnuToolsAccountsConfig = new wxMenuItem( mnuTools, wxID_ANY, wxString( wxT("Accounts Configuration...") ) , wxEmptyString, wxITEM_NORMAL );
	mnuTools->Append( mnuToolsAccountsConfig );

	mnubarMain->Append( mnuTools, wxT("Tools") );

	mnuHelp = new wxMenu();
	wxMenuItem* mnuHelpAbout;
	mnuHelpAbout = new wxMenuItem( mnuHelp, wxID_ANY, wxString( wxT("About...") ) , wxEmptyString, wxITEM_NORMAL );
	mnuHelp->Append( mnuHelpAbout );

	mnubarMain->Append( mnuHelp, wxT("?") );

	this->SetMenuBar( mnubarMain );

	tbarMain = this->CreateToolBar( wxTB_FLAT|wxTB_HORIZONTAL|wxBORDER_RAISED, wxID_ANY );
	tbarMain->SetToolSeparation( 4 );
	toolEditAction1 = tbarMain->AddTool( wxID_ANY, wxEmptyString, wxArtProvider::GetBitmap( wxASCII_STR("IcoToolPrint"), wxASCII_STR(wxART_OTHER) ), wxNullBitmap, wxITEM_NORMAL, wxT("Action 1..."), wxEmptyString, NULL );

	toolEditAction2 = tbarMain->AddTool( wxID_ANY, wxEmptyString, wxArtProvider::GetBitmap( wxASCII_STR("IcoToolPrint"), wxASCII_STR(wxART_OTHER) ), wxNullBitmap, wxITEM_NORMAL, wxT("Action 2..."), wxEmptyString, NULL );

	tbarMain->Realize();

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	tabCtrlMain = new wxAuiNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_DEFAULT_STYLE );

	bSizer1->Add( tabCtrlMain, 1, wxEXPAND, 2 );


	this->SetSizer( bSizer1 );
	this->Layout();
	sbarMain = this->CreateStatusBar( 1, wxSTB_DEFAULT_STYLE, wxID_ANY );

	this->Centre( wxBOTH );

	// Connect Events
	mnuFile->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainWndUI::mnuFileExit_OnMenuSelection ), this, mnuFileExit->GetId());
	mnuEdit->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainWndUI::mnuEditNewMailMessageOnMenuSelection ), this, mnuEditNewMailMessage->GetId());
	mnuView->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainWndUI::mnuViewToolbar_OnMenuSelection ), this, mnuViewToolbar->GetId());
	mnuView->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainWndUI::mnuViewStatusBar_OnMenuSelection ), this, mnuViewStatusBar->GetId());
	mnuView->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainWndUI::mnuViewLog_OnMenuSelection ), this, mnuViewLog->GetId());
	mnuTools->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainWndUI::mnuToolsAccountsConfig_OnMenuSelection ), this, mnuToolsAccountsConfig->GetId());
	mnuHelp->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MainWndUI::mnuHelpAbout_OnMenuSelection ), this, mnuHelpAbout->GetId());
	this->Connect( toolEditAction1->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainWndUI::toolEditAction1_OnToolClicked ) );
	this->Connect( toolEditAction2->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainWndUI::toolEditAction2_OnToolClicked ) );
	tabCtrlMain->Connect( wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler( MainWndUI::tabCtrlMain_OnAuiNotebookPageChanged ), NULL, this );
}

MainWndUI::~MainWndUI()
{
	// Disconnect Events
	this->Disconnect( toolEditAction1->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainWndUI::toolEditAction1_OnToolClicked ) );
	this->Disconnect( toolEditAction2->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MainWndUI::toolEditAction2_OnToolClicked ) );
	tabCtrlMain->Disconnect( wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler( MainWndUI::tabCtrlMain_OnAuiNotebookPageChanged ), NULL, this );

}
