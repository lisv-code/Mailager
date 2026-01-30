#include "PasswordStore.h"
#include <cstring>
#include <LisCommon/StrUtils.h>
#include "AppDef.h"
#include "SecretStore.h"
#include "AppResCodes.h"

namespace PasswordStore_Imp
{
#define Log_Scope "PswdStor"
}
using namespace PasswordStore_Imp;
using namespace LisLog;

std::string PasswordStore::GetStoreKey(const char* host, const char* user)
{
	return std::string(host) + "/" + user;
}

PasswordStore::PasswordStore()
{
	SecretStoreSettings sss{};
	sss.KeyGroup = _TEXT("" AppDef_PswdStoreGroup);
	// TODO: provide the path to the SecretStore file
	// sss.FilePath = ...;
	ss = SecretStore::CreateInstance(sss);
}

int PasswordStore::LoadPassword(const char* host, const char* user, std::string& pswd)
{
	if (ss->Load(GetStoreKey(host, user).c_str(), pswd))
		return ResCode_Ok;
	else {
		logger->LogTxt(llWarn, Log_Scope " Password couldn't be loaded.");
		return Error_Gen_Undefined;
	}
}

int PasswordStore::SavePassword(const char* host, const char* user, const char* pswd)
{
	if (ss->Store(GetStoreKey(host, user).c_str(), pswd, std::strlen(pswd)))
		return ResCode_Ok;
	else {
		logger->LogTxt(llWarn, Log_Scope " Password couldn't be saved.");
		return Error_Gen_Undefined;
	}
}
