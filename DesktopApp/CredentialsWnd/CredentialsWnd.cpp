#include "CredentialsWnd.h"
#include "../AppMgr.h"
#include "../UiHelper.h"

namespace CredentialsWnd_Imp
{
#define InfoMsg "Credentials for %s"
}
using namespace CredentialsWnd_Imp;

CredentialsWnd::CredentialsWnd(wxWindow* Parent) :CredentialsWndUI(Parent)
{
	InitUI();
}

CredentialsWnd::~CredentialsWnd() { }

void CredentialsWnd::InitUI()
{
	UiHelper::InitDialog(this);
	txtPswd->SetFocus();
}

void CredentialsWnd::SetData(const char* server, const char* user, const char* pswd, bool need_save)
{
	lblInfo->SetLabelText(wxString::Format(InfoMsg, server));
	txtUser->SetValue(user);
	txtPswd->SetValue(pswd);
	chkSave->SetValue(need_save);
}

void CredentialsWnd::GetData(std::string& pswd, bool* need_save)
{
	pswd = txtPswd->GetValue().ToUTF8().data();
	if (need_save)
		*need_save = chkSave->GetValue();
}
