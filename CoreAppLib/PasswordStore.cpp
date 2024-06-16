#include "PasswordStore.h"
#include <wx/secretstore.h> // requires libsecret under Unix
#include <LisCommon/StrUtils.h>
#include "AppDef.h"

namespace AuthStorePswd_Imp
{
#define Err_StoreInitFailure -2
#define Err_StoreOperFailure -3
#define Log_Scope "PswdStor"
}
using namespace AuthStorePswd_Imp;
using namespace LisLog;

std::string PasswordStore::GetStoreKey(const char* host, const char* user)
{
	return std::string(AppDef_PswdStorePrefix) + host + "/" + user;
}

int PasswordStore::LoadPassword(const char* key, std::string* user, std::string& pswd)
{
	wxSecretStore store = wxSecretStore::GetDefault();
	wxString errmsg;
	if (store.IsOk(&errmsg)) {
		wxString user_name;
		wxSecretValue pswd_data;
		if (store.Load(key, user_name, pswd_data))
		{
			if (user) *user = user_name.ToStdString();
#ifdef _WINDOWS
			pswd = pswd_data.GetAsString(
				sizeof(TCHAR) > sizeof(char) ? (wxMBConv&)wxMBConvUTF16LE() : (wxMBConv&)wxMBConvUTF8()
			).ToStdString();
#else
			pswd = pswd_data.GetAsString(wxMBConvUTF8()).ToStdString();
#endif
			return 0;
		}
		logger->LogTxt(llWarn, Log_Scope " Password can't be loaded.");
		return Err_StoreOperFailure;
	} else {
		logger->LogFmt(llError,
			Log_Scope " The system doesn't support storing passwords securely: %s", errmsg);
		return Err_StoreInitFailure;
	}
}

int PasswordStore::SavePassword(const char* key, const char* user, const char* pswd)
{
	int result = 0;
	wxSecretStore store = wxSecretStore::GetDefault();
	wxString errmsg;
	if (store.IsOk(&errmsg)) {
#ifdef _WINDOWS
		auto secret = wxSecretValue(strlen(pswd) * sizeof(TCHAR),
			(TCHAR*)LisStr::CStrConvert(pswd, CP_UTF8));
#else
		auto secret = wxSecretValue(pswd);
#endif
		if (!store.Save(key, user, secret)) {
			result = Err_StoreOperFailure;
			logger->LogTxt(llError, Log_Scope " Failed saving credentials to the system secret store.");
		}
	} else {
		result = Err_StoreInitFailure;
		logger->LogFmt(llError,
			Log_Scope " The system doesn't support storing passwords securely: %s", errmsg);
	}
	return result;
}
