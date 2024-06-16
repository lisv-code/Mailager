#pragma once
#include <string>
#include <vector>
#include <wx/confbase.h>
#include <LisCommon/EventDispBase.h>
#include <LisCommon/FileSystem.h>
#include "AccountSettings.h"

class AccountCfg; // forward declaration

namespace AccountCfg_Def
{
	enum EventType { etAccountsChanged };
	struct EventData {
		std::vector<const AccountSettings*> Accounts;
		std::vector<int> DeletedAccIds;
	};
	typedef EventDispatcherBase<AccountCfg, EventType, const EventData*> AccountCfg_EventDispatcher;
}

class AccountCfg : public AccountCfg_Def::AccountCfg_EventDispatcher
{
public:
	typedef std::vector<AccountSettings> AccountsContainer;
	typedef AccountsContainer::const_iterator AccountsIterator;
private:
	std::basic_string<FILE_PATH_CHAR> filePath;
	unsigned int accLastId;
	AccountsContainer accounts;
public:
	AccountCfg();
	void SetFilePath(const FILE_PATH_CHAR* file_path);
	int Load();
	int Save(AccountSettings* save_items, size_t save_count, int* del_ids, size_t del_count);
	unsigned int GetLastId() const;
	void GetIter(AccountsIterator& begin, AccountsIterator& end) const;
	const AccountSettings* FindAccount(int acc_id) const;
};

extern AccountCfg AccCfg; // Account Configuration global singleton
