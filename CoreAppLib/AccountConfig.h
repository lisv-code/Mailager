#pragma once
#include <string>
#include <vector>
#include <utility>
#include <wx/confbase.h>
#include <LisCommon/EventDispBase.h>
#include <LisCommon/FileSystem.h>
#include "AccountSettings.h"

class AccountConfig; // forward declaration

enum class AccountConfig_EventType { AccountsChanged };
struct AccountConfig_EventData {
	std::vector<const AccountSettings*> Accounts;
	std::vector<int> DeletedAccIds;
};
typedef EventDispatcherBase<AccountConfig, AccountConfig_EventType, const AccountConfig_EventData&> AccountConfig_EvtDisp;

class AccountConfig : public AccountConfig_EvtDisp
{
public:
	typedef std::vector<AccountSettings> AccountsContainer;
	typedef AccountsContainer::const_iterator AccountsIterator;

	AccountConfig();

	const FILE_PATH_CHAR* GetCfgPath() const;
	void SetCfgPath(const FILE_PATH_CHAR* file_path);
	int Load();
	int Save(AccountSettings* save_items, size_t save_count, int* del_ids, size_t del_count);

	unsigned int GetLastId() const;
	std::pair<AccountsIterator, AccountsIterator> GetIter() const;
	const AccountSettings* FindAccount(int acc_id) const;

private:
	std::basic_string<FILE_PATH_CHAR> filePath;
	unsigned int accLastId;
	AccountsContainer accounts;
};

extern AccountConfig AccCfg; // Account Configuration global singleton
