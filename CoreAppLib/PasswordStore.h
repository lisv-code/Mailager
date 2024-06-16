#pragma once
#include <string>
#include <LisCommon/Logger.h>

class PasswordStore
{
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
public:
	static std::string GetStoreKey(const char* host, const char* user);

	int LoadPassword(const char* key, std::string* user, std::string& pswd);
	int SavePassword(const char* key, const char* user, const char* pswd);
};
