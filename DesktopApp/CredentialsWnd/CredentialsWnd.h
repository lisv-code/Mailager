#pragma once
#include <string>
#include "CredentialsWndUI.h"

class CredentialsWnd: public CredentialsWndUI
{
	void InitUI();
public:
	CredentialsWnd(wxWindow* Parent);
	virtual ~CredentialsWnd();

	void SetData(const char* server, const char* user, const char* pswd, bool need_save);
	void GetData(std::string& pswd, bool* need_save);
};
