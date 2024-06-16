#pragma once
#include "MainWndUI.h"
#include <LisCommon/Logger.h>
#include "../MailMsgFileMgr.h"
#include "../MailMsgViewMgr.h"

class MainWnd: public MainWndUI
{
	virtual void mnuFileExit_OnMenuSelection(wxCommandEvent& event) override;
	virtual void mnuEditNewMailMessageOnMenuSelection(wxCommandEvent& event) override;
	virtual void mnuViewToolbar_OnMenuSelection(wxCommandEvent& event) override;
	virtual void mnuViewLog_OnMenuSelection(wxCommandEvent& event) override;
	virtual void mnuToolsAccountsConfig_OnMenuSelection(wxCommandEvent& event) override;
	virtual void mnuHelpAbout_OnMenuSelection(wxCommandEvent& event) override;
	virtual void toolEditAction1_OnToolClicked(wxCommandEvent& event) override;
	virtual void toolEditAction2_OnToolClicked(wxCommandEvent& event) override;
	virtual void tabCtrlMain_OnAuiNotebookPageChanged(wxAuiNotebookEvent& event) override;

	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	wxWindow* wndTabFixed;
	MailMsgFileMgr mailMsgFileMgr;
	MailMsgViewMgr mailMsgViewMgr;
	void UpdateMainViewCreated(wxWindow* window, const wxString& title, bool fixed = false);
	void CreateMailMainView();
public:
	MainWnd(wxWindow* parent);
	~MainWnd();
};
