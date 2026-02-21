#pragma once
#include <string>
#include <unordered_map>
#include "../../CoreAppLib/AccountSettings.h"
#include "../../CoreAppLib/ListEditMgr.h"

class AccEditMgr
{
public:
	AccEditMgr();
	// Loads config data and returns account IDs with names
	std::unordered_map<int, std::string> LoadData();

	AccountSettings* FindAccount(int acc_id);
	bool SetAccountModified(int acc_id);
	int CreateAccount();
	bool DeleteAccount(int acc_id);
	std::unordered_map<ListEditMgr_Def::EditState, size_t> GetAccEditState() const;

	bool ApplyChanges();

private:
	ListEditMgr<AccountSettings, int> editMgr;
};
