#pragma once
#include <memory>
#include <string>
#include <LisCommon/Logger.h>
#include "SecretStore.h"

class PasswordStore
{
public:
	PasswordStore();

	int LoadPassword(const char* host, const char* user, std::string& pswd);
	int SavePassword(const char* host, const char* user, const char* pswd);
private:
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	std::unique_ptr<SecretStore> ss;

	static std::string GetStoreKey(const char* host, const char* user);
};
