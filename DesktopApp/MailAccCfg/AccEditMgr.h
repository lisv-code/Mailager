#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <LisCommon/Logger.h>
#include "../../CoreAppLib/AccountSettings.h"

class AccEditMgr
{
public:
	enum EditState { esNone = 0, esCreated, esModified, esDeleted };
private:
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	std::vector<std::pair<AccountSettings, EditState>> accounts;

	int FindAccIdx(int acc_id);

	int InitAccResources(const AccountSettings& acc);
	int DeleteAccResources(const AccountSettings& acc);
	int SaveAccounts();
public:
	std::unordered_map<int, std::string> LoadAccounts();
	AccountSettings* FindAccount(int acc_id);

	bool SetAccountModified(int acc_id);
	int CreateAccount();
	void DeleteAccount(int acc_id);

	std::unordered_map<EditState, int> GetEditState();
	int ApplyChanges();
};
