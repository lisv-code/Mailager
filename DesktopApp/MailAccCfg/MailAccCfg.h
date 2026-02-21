#pragma once
#include "MailAccCfgUI.h"
#include "AccEditMgr.h"

class MailAccCfg: public MailAccCfgUI
{
	int curSelAccIdx;
	AccEditMgr accMgr;

	void InitUI();
	void LoadAccounts();
	AccountSettings* FindAccount(int sel_idx);
	std::vector<wxString> ValidateAccData(const AccountSettings& acc);
	void LoadAccViewData(int sel_idx);
	std::vector<wxString> ValidateAccViewData(const AccountSettings* acc);
	void ShowAccValidationErrors(const AccountSettings* acc, const std::vector<wxString>& errors);
	bool CheckAndSaveAccViewData(int sel_idx);
	bool GetChangeInfo(wxString& info);
	void ApplyChanges();

	virtual void MailAccCfgUI_OnInitDialog(wxInitDialogEvent& event) override;
	virtual void toolAccCreate_OnToolClicked(wxCommandEvent& event) override;
	virtual void toolAccDelete_OnToolClicked(wxCommandEvent& event) override;
	virtual void chcAccount_OnChoice(wxCommandEvent& event) override;
	virtual void btnOk_OnButtonClick(wxCommandEvent& event) override;
	virtual void btnNo_OnButtonClick(wxCommandEvent& event) override;
	virtual void MailAccCfgUI_OnClose(wxCloseEvent& event) override;
public:
	MailAccCfg(wxWindow* Parent);
	~MailAccCfg();
};
