///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "MailMainViewUI.h"

///////////////////////////////////////////////////////////////////////////

MailMainViewUI::MailMainViewUI( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxPanel( parent, id, pos, size, style, name )
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );

	wndMasterSplitter = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
	wndMasterSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( MailMainViewUI::wndMasterSplitterOnIdle ), NULL, this );
	wndMasterSplitter->SetMinimumPaneSize( 60 );

	pnlMasterView = new wxPanel( wndMasterSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxVERTICAL );

	tlbrMaster = new wxToolBar( pnlMasterView, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_HORIZONTAL|wxBORDER_RAISED );
	tlbrMaster->SetToolSeparation( 4 );
	toolMasterViewConfig = tlbrMaster->AddTool( wxID_ANY, wxT("config"), wxArtProvider::GetBitmap( wxASCII_STR("IcoToolConfig"), wxASCII_STR(wxART_OTHER) ), wxNullBitmap, wxITEM_NORMAL, wxT("Configure main view"), wxEmptyString, NULL );

	toolMailSyncProc = tlbrMaster->AddTool( wxID_ANY, wxT("sync"), wxArtProvider::GetBitmap( wxASCII_STR("IcoToolRefresh"), wxASCII_STR(wxART_OTHER) ), wxNullBitmap, wxITEM_NORMAL, wxT("Start mail sync"), wxEmptyString, NULL );

	tlbrMaster->AddSeparator();

	toolMailMsgCreate = tlbrMaster->AddTool( wxID_ANY, wxT("newmsg"), wxArtProvider::GetBitmap( wxASCII_STR("IcoToolEdit"), wxASCII_STR(wxART_OTHER) ), wxNullBitmap, wxITEM_NORMAL, wxT("Create mail message"), wxEmptyString, NULL );

	tlbrMaster->Realize();

	bSizer3->Add( tlbrMaster, 0, wxEXPAND, 4 );

	dvAccFolders = new wxDataViewCtrl( pnlMasterView, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_NO_HEADER );
	dvcAccFoldersCol0 = dvAccFolders->AppendTextColumn( wxT("Name"), 0, wxDATAVIEW_CELL_INERT, -1, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE );
	dvcAccFoldersCol1 = dvAccFolders->AppendTextColumn( wxT("Count"), 1, wxDATAVIEW_CELL_INERT, -1, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE );
	bSizer3->Add( dvAccFolders, 1, wxALL|wxEXPAND, 2 );


	pnlMasterView->SetSizer( bSizer3 );
	pnlMasterView->Layout();
	bSizer3->Fit( pnlMasterView );
	pnlDetailView = new wxPanel( wndMasterSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxVERTICAL );

	wndDetailSplitter = new wxSplitterWindow( pnlDetailView, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
	wndDetailSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( MailMainViewUI::wndDetailSplitterOnIdle ), NULL, this );
	wndDetailSplitter->SetMinimumPaneSize( 20 );

	pnlMailMsgList = new wxPanel( wndDetailSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxVERTICAL );

	tlbrDetail = new wxToolBar( pnlMailMsgList, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_HORIZONTAL|wxBORDER_RAISED );
	tlbrDetail->SetToolSeparation( 4 );
	toolMailMsgFilterSwitch = tlbrDetail->AddTool( wxID_ANY, wxT("filter1"), wxArtProvider::GetBitmap( wxASCII_STR("IcoToolView"), wxASCII_STR(wxART_OTHER) ), wxNullBitmap, wxITEM_CHECK, wxT("Filter mail messages"), wxEmptyString, NULL );

	cmbMailMsgFilterValue = new wxComboBox( tlbrDetail, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxTE_PROCESS_ENTER );
	tlbrDetail->AddControl( cmbMailMsgFilterValue );
	toolMailMsgFilterApply = tlbrDetail->AddTool( wxID_ANY, wxT("filter2"), wxArtProvider::GetBitmap( wxASCII_STR("IcoBtnOk"), wxASCII_STR(wxART_OTHER) ), wxNullBitmap, wxITEM_NORMAL, wxT("Apply filter"), wxEmptyString, NULL );

	toolMailMsgLayout = tlbrDetail->AddTool( wxID_ANY, wxT("layout"), wxArtProvider::GetBitmap( wxASCII_STR("IcoToolLayout"), wxASCII_STR(wxART_OTHER) ), wxNullBitmap, wxITEM_NORMAL, wxT("Switch view layout"), wxEmptyString, NULL );

	tlbrDetail->Realize();

	bSizer7->Add( tlbrDetail, 0, wxEXPAND, 4 );

	dvMailMsgList = new wxDataViewCtrl( pnlMailMsgList, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_MULTIPLE|wxDV_NO_HEADER|wxDV_ROW_LINES );
	dvcDetailCol0 = dvMailMsgList->AppendTextColumn( wxT("Name"), 0, wxDATAVIEW_CELL_INERT, -1, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE );
	bSizer7->Add( dvMailMsgList, 1, wxALL|wxEXPAND, 2 );


	pnlMailMsgList->SetSizer( bSizer7 );
	pnlMailMsgList->Layout();
	bSizer7->Fit( pnlMailMsgList );
	mnuMailMsgItem = new wxMenu();
	wxMenuItem* mnuMailMsgItemOpen;
	mnuMailMsgItemOpen = new wxMenuItem( mnuMailMsgItem, wxID_ANY, wxString( wxT("Open in new tab") ) , wxEmptyString, wxITEM_NORMAL );
	mnuMailMsgItem->Append( mnuMailMsgItemOpen );

	mnuMailMsgItem->AppendSeparator();

	wxMenuItem* mnuMailMsgItemMarkAsRead;
	mnuMailMsgItemMarkAsRead = new wxMenuItem( mnuMailMsgItem, wxID_ANY, wxString( wxT("Mark as Read") ) , wxEmptyString, wxITEM_NORMAL );
	mnuMailMsgItem->Append( mnuMailMsgItemMarkAsRead );

	wxMenuItem* mnuMailMsgItemMarkUnread;
	mnuMailMsgItemMarkUnread = new wxMenuItem( mnuMailMsgItem, wxID_ANY, wxString( wxT("Mark Unread") ) , wxEmptyString, wxITEM_NORMAL );
	mnuMailMsgItem->Append( mnuMailMsgItemMarkUnread );

	mnuMailMsgItem->AppendSeparator();

	wxMenuItem* mnuMailMsgItemDelete;
	mnuMailMsgItemDelete = new wxMenuItem( mnuMailMsgItem, wxID_ANY, wxString( wxT("Delete...") ) , wxEmptyString, wxITEM_NORMAL );
	mnuMailMsgItem->Append( mnuMailMsgItemDelete );

	pnlMailMsgList->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( MailMainViewUI::pnlMailMsgListOnContextMenu ), NULL, this );

	pnlMailMsgView = new wxPanel( wndDetailSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wndDetailSplitter->SplitVertically( pnlMailMsgList, pnlMailMsgView, 360 );
	bSizer5->Add( wndDetailSplitter, 1, wxEXPAND, 2 );


	pnlDetailView->SetSizer( bSizer5 );
	pnlDetailView->Layout();
	bSizer5->Fit( pnlDetailView );
	wndMasterSplitter->SplitVertically( pnlMasterView, pnlDetailView, 220 );
	bSizer1->Add( wndMasterSplitter, 1, wxEXPAND, 2 );


	this->SetSizer( bSizer1 );
	this->Layout();

	// Connect Events
	this->Connect( toolMasterViewConfig->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailMainViewUI::toolMasterViewConfig_OnToolClicked ) );
	this->Connect( toolMailMsgCreate->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailMainViewUI::toolMailMsgCreate_OnToolClicked ) );
	dvAccFolders->Connect( wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler( MailMainViewUI::dvAccFolders_OnDataViewCtrlSelectionChanged ), NULL, this );
	this->Connect( toolMailMsgFilterSwitch->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailMainViewUI::toolMailMsgFilterSwitch_OnToolClicked ) );
	cmbMailMsgFilterValue->Connect( wxEVT_KEY_DOWN, wxKeyEventHandler( MailMainViewUI::cmbMailMsgFilterValue_OnKeyDown ), NULL, this );
	cmbMailMsgFilterValue->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( MailMainViewUI::cmbMailMsgFilterValue_OnText ), NULL, this );
	cmbMailMsgFilterValue->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( MailMainViewUI::cmbMailMsgFilterValue_OnTextEnter ), NULL, this );
	this->Connect( toolMailMsgFilterApply->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailMainViewUI::toolMailMsgFilterApply_OnToolClicked ) );
	this->Connect( toolMailMsgLayout->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailMainViewUI::toolMailMsgLayout_OnToolClicked ) );
	dvMailMsgList->Connect( wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED, wxDataViewEventHandler( MailMainViewUI::dvMailMsgList_OnDataViewCtrlItemActivated ), NULL, this );
	dvMailMsgList->Connect( wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, wxDataViewEventHandler( MailMainViewUI::dvMailMsgList_OnDataViewCtrlItemContextMenu ), NULL, this );
	dvMailMsgList->Connect( wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler( MailMainViewUI::dvMailMsgList_OnDataViewCtrlSelectionChanged ), NULL, this );
	mnuMailMsgItem->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MailMainViewUI::mnuMailMsgItemOpen_OnMenuSelection ), this, mnuMailMsgItemOpen->GetId());
	mnuMailMsgItem->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MailMainViewUI::mnuMailMsgItemMarkAsRead_OnMenuSelection ), this, mnuMailMsgItemMarkAsRead->GetId());
	mnuMailMsgItem->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MailMainViewUI::mnuMailMsgItemMarkUnread_OnMenuSelection ), this, mnuMailMsgItemMarkUnread->GetId());
	mnuMailMsgItem->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( MailMainViewUI::mnuMailMsgItemDelete_OnMenuSelection ), this, mnuMailMsgItemDelete->GetId());
}

MailMainViewUI::~MailMainViewUI()
{
	// Disconnect Events
	this->Disconnect( toolMasterViewConfig->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailMainViewUI::toolMasterViewConfig_OnToolClicked ) );
	this->Disconnect( toolMailMsgCreate->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailMainViewUI::toolMailMsgCreate_OnToolClicked ) );
	dvAccFolders->Disconnect( wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler( MailMainViewUI::dvAccFolders_OnDataViewCtrlSelectionChanged ), NULL, this );
	this->Disconnect( toolMailMsgFilterSwitch->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailMainViewUI::toolMailMsgFilterSwitch_OnToolClicked ) );
	cmbMailMsgFilterValue->Disconnect( wxEVT_KEY_DOWN, wxKeyEventHandler( MailMainViewUI::cmbMailMsgFilterValue_OnKeyDown ), NULL, this );
	cmbMailMsgFilterValue->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( MailMainViewUI::cmbMailMsgFilterValue_OnText ), NULL, this );
	cmbMailMsgFilterValue->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( MailMainViewUI::cmbMailMsgFilterValue_OnTextEnter ), NULL, this );
	this->Disconnect( toolMailMsgFilterApply->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailMainViewUI::toolMailMsgFilterApply_OnToolClicked ) );
	this->Disconnect( toolMailMsgLayout->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler( MailMainViewUI::toolMailMsgLayout_OnToolClicked ) );
	dvMailMsgList->Disconnect( wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED, wxDataViewEventHandler( MailMainViewUI::dvMailMsgList_OnDataViewCtrlItemActivated ), NULL, this );
	dvMailMsgList->Disconnect( wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, wxDataViewEventHandler( MailMainViewUI::dvMailMsgList_OnDataViewCtrlItemContextMenu ), NULL, this );
	dvMailMsgList->Disconnect( wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler( MailMainViewUI::dvMailMsgList_OnDataViewCtrlSelectionChanged ), NULL, this );

	delete mnuMailMsgItem;
}
