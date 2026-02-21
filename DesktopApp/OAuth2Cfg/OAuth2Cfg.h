#pragma once
#include "OAuth2CfgUI.h"
#include "OAuth2EditMgr.h"

class OAuth2Cfg : public OAuth2CfgUI
{
public:
	OAuth2Cfg(wxWindow* parent);
	~OAuth2Cfg();
private:
	int curSelProvIdx;
	OAuth2EditMgr edMgr;
	void InitUI();
	void LoadData();
	OAuth2ProviderSettings* FindProvider(int sel_idx);
	std::vector<wxString> ValidateProvData(const OAuth2ProviderSettings& prov);
	void LoadProvViewData(int sel_idx);
	std::vector<wxString> ValidateProvViewData(const OAuth2ProviderSettings* prov);
	void ShowProvValidationErrors(const OAuth2ProviderSettings* prov, const std::vector<wxString>& errors);
	bool CheckAndSaveProvViewData(int sel_idx);
	bool GetChangeInfo(wxString& info);
	void ApplyChanges();

	virtual void OAuth2CfgUI_OnInitDialog(wxInitDialogEvent& event) override;
	virtual void toolProvCreate_OnToolClicked(wxCommandEvent& event) override;
	virtual void toolProvDelete_OnToolClicked(wxCommandEvent& event) override;
	virtual void lbxProviders_OnListBox(wxCommandEvent& event) override;
	virtual void btnOk_OnButtonClick(wxCommandEvent& event) override;
	virtual void btnNo_OnButtonClick(wxCommandEvent& event) override;
	virtual void OAuth2CfgUI_OnClose(wxCloseEvent& event) override;
};
