﻿#include "MainWnd.h"
#include <string>
#include <LisCommon/StrUtils.h>
#include "../../CoreAppLib/AppDef.h"
#include "../../CoreAppLib/MailMsgFileDef.h"
#include "../ResMgr.h"
#include "../MailMainView/MailMainView.h"
#include "../LogView/LogView.h"
#include "../MailAccCfg/MailAccCfg.h"

#define Res_AppMainIcon "IcoAppMain"
#define Log_Scope "DspMain"
#define SBar_FldIdx_TxtLog 0

wxDEFINE_EVENT(LOG_WRITE_EVENT, wxCommandEvent);

MainWnd::MainWnd(wxWindow* parent) : MainWndUI(parent)
{
	wndTabFixed = nullptr;

	mailMsgViewMgr.SetStdViewDefaults(tabCtrlMain, static_cast<IMailMsgViewCtrl*>(this));

	SetIcon(ResMgr::GetIcon(_T(Res_AppMainIcon)));
	//this->SetLabel(this->GetLabel() + wxT(" ") + DataModule::Instance.GetAppVer());

	tbarMain->Hide(); // TODO: the main toolbar visibility to be configurable
	tabCtrlMain->SetTabCtrlHeight(0); // Set height to zero when child windows count less than 2

	//wxConfigBase* cfg = DataModule::Instance.GetCfg();
	//iStateTimeOut = cfg->ReadLong(wxCONCAT3("Interface", "/", "StateTimeOut"), 0);

	CreateMailMainView();

	LogInit(true);

	auto data = ResMgr::GetVersionInfo();
	logger->LogFmt(LisLog::llInfo, Log_Scope " application started %s.",
		(char*)LisStr::CStrConvert(data.Version));
#ifndef NDEBUG
	logger->LogTxt(LisLog::llInfo, Log_Scope " DEBUG build.");
#endif
}

MainWnd::~MainWnd()
{
	// tabCtrlMain->DeleteAllPages(); // Views are freed before the MailMsgFile collection
	LogInit(false);
}

void MainWnd::UpdateMain_ViewCreated(wxWindow* window, const wxString& title, bool fixed)
{
	tabCtrlMain->AddPage(window, title, true);
	if (fixed) wndTabFixed = window;
	window->Show();
}

void MainWnd::CreateMailMainView()
{
	MailMainView* wnd = new MailMainView(tabCtrlMain, &mailMsgFileMgr, &mailMsgViewMgr);
	UpdateMain_ViewCreated(wnd, MailMainView_Def::WndTitle, true);
}

void MainWnd::mnuFileExit_OnMenuSelection(wxCommandEvent& event) { this->Close(); }

void MainWnd::mnuEditNewMailMessageOnMenuSelection(wxCommandEvent& event)
{
	mailMsgViewMgr.OpenStdView(mailMsgFileMgr.CreateMailMsg(MailMsgGrpId_Empty));
}

void MainWnd::mnuViewToolbar_OnMenuSelection(wxCommandEvent& event)
{
	if (tbarMain->IsShown()) tbarMain->Hide(); else { tbarMain->Show(); tbarMain->Realize(); }
	this->Layout();
}

void MainWnd::mnuViewStatusBar_OnMenuSelection(wxCommandEvent& event)
{
	if (sbarMain->IsShown()) { sbarMain->Hide(); LogInit(false); }
	else { sbarMain->Show(); LogInit(true); }
	this->SendSizeEvent();
}

void MainWnd::mnuViewLog_OnMenuSelection(wxCommandEvent& event)
{
	UpdateMain_ViewCreated(new LogView(tabCtrlMain), LogView_Def::WndTitle, false);
}

void MainWnd::mnuToolsAccountsConfig_OnMenuSelection(wxCommandEvent& event)
{
	MailAccCfg wnd(this);
	wnd.ShowModal();
}

void MainWnd::toolEditAction1_OnToolClicked(wxCommandEvent& event)
{
	// mnuEditAction1_OnMenuSelection(event);
}

void MainWnd::toolEditAction2_OnToolClicked(wxCommandEvent& event)
{
	// mnuEditAction2_OnMenuSelection(event);
}

void MainWnd::tabCtrlMain_OnAuiNotebookPageChanged(wxAuiNotebookEvent& event)
{
	if (!wndTabFixed) return;
	// Hide TabCtrl if just a single tab presented
	if (tabCtrlMain->GetPageCount() < 2) {
		tabCtrlMain->SetTabCtrlHeight(0);
		return;
	} else tabCtrlMain->SetTabCtrlHeight(-1);
	// Hide a close button if a "fixed" tab selected, show it otherwise
	int tab_idx;
	wxAuiTabCtrl* tab_ctrl_x = nullptr;
	bool has_tab = tabCtrlMain->FindTab(wndTabFixed, &tab_ctrl_x, &tab_idx);
	if (has_tab) {
		if (tab_ctrl_x->GetActivePage() == tab_idx) {
			tab_ctrl_x->SetFlags(tab_ctrl_x->GetFlags() & ~wxAUI_NB_CLOSE_ON_ACTIVE_TAB);
		} else {
			tab_ctrl_x->SetFlags(tab_ctrl_x->GetFlags() | wxAUI_NB_CLOSE_ON_ACTIVE_TAB);
		}
	}
	wxAuiTabCtrl* tab_ctrl_0 = tabCtrlMain->GetActiveTabCtrl();
	if (tab_ctrl_0 != tab_ctrl_x) {
		tab_ctrl_0->SetFlags(tab_ctrl_0->GetFlags() | wxAUI_NB_CLOSE_ON_ACTIVE_TAB);
	}
}

void MainWnd::LogInit(bool enable)
{
	if (enable) {
		sbarMain->SetStatusText("");
		Bind(LOG_WRITE_EVENT, &MainWnd::LogWriteEventHandler, this);
		logTarget = logger->AddTarget(new LisLog::LogTargetTextFunc(
			[this](LisLog::LogTargetBase::EventType type, const char* txt) { LogTargetFunc(txt); },
			LisLog::llInfo, false));
	} else {
		Unbind(LOG_WRITE_EVENT, &MainWnd::LogWriteEventHandler, this);
		if (logTarget != nullptr) {
			logger->DelTarget(logTarget);
			logTarget = nullptr;
		}
	}
}

void MainWnd::LogTargetFunc(const char* txt)
{
	auto evt = new wxCommandEvent(LOG_WRITE_EVENT);
	evt->SetString(txt);
	wxQueueEvent(this, evt);
}

void MainWnd::LogWriteEventHandler(wxCommandEvent& event)
{
	sbarMain->SetStatusText(event.GetString(), SBar_FldIdx_TxtLog);
}

#include <wx/aboutdlg.h>

void MainWnd::mnuHelpAbout_OnMenuSelection(wxCommandEvent& event)
{
	wxAboutDialogInfo dlg;
	dlg.SetIcon(ResMgr::GetIcon(wxT(Res_AppMainIcon)));
	dlg.SetName(wxT(AppDef_Title));
	auto data = ResMgr::GetVersionInfo();
	dlg.SetVersion(data.Version);
	dlg.SetCopyright(data.Copyright);
	dlg.SetDescription(data.Comments);
	dlg.SetWebSite(wxT("http://lisv.site")); // TODO: Should be path to the app page
	wxAboutBox(dlg, this);
}

// *************************************** IMailMsgViewCtrl ****************************************

void MainWnd::OnViewCreated(wxWindow* wnd, const TCHAR* title)
{
	UpdateMain_ViewCreated(wnd, title, false);
}

wxWindow* MainWnd::GetView(int index)
{
	return tabCtrlMain->GetPageCount() > index ? tabCtrlMain->GetPage(index) : nullptr;
}

void MainWnd::ActivateView(int index)
{
	tabCtrlMain->SetSelection(index);
}
