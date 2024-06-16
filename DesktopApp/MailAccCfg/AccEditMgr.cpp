#include "AccEditMgr.h"
#include "../../CoreAppLib/AccountCfg.h"
#include "../../CoreAppLib/MailMsgStore.h"
#include "../AppCfg.h"

#define DefaultAccDirNamePrefix "account"
#define Log_Scope "AccEdit"

using namespace LisLog;

int AccEditMgr::FindAccIdx(int acc_id)
{
	for (size_t i = 0; i < accounts.size(); ++i) {
		if (accounts[i].first.Id == acc_id) {
			return i;
		}
	}
	return -1;
}

std::unordered_map<int, std::string> AccEditMgr::LoadAccounts()
{
	std::unordered_map<int, std::string> result;
	AccountCfg::AccountsIterator acc_list_begin, acc_list_end;
	AccCfg.GetIter(acc_list_begin, acc_list_end);
	for (auto it = acc_list_begin; it != acc_list_end; ++it) {
		accounts.push_back(std::make_pair(*it, esNone));
		auto& acc = accounts.back().first;
		result.insert(std::make_pair(acc.Id, acc.GetName()));
	}
	return result;
}

AccountSettings* AccEditMgr::FindAccount(int acc_id)
{
	int idx = FindAccIdx(acc_id);
	return idx >= 0 ? &accounts[idx].first : nullptr;
}

bool AccEditMgr::SetAccountModified(int acc_id)
{
	int idx = FindAccIdx(acc_id);
	if ((idx >= 0) && (esNone == accounts[idx].second)) {
		accounts[idx].second = esModified;
		return true;
	}
	return false;
}

int AccEditMgr::CreateAccount()
{
	int acc_id = AccCfg.GetLastId();
	for (const auto& acc : accounts) if (acc_id < acc.first.Id) acc_id = acc.first.Id;
	++acc_id;

	std::string acc_inf = DefaultAccDirNamePrefix;
	acc_inf += std::to_string(acc_id);
	accounts.push_back(std::make_pair(AccountSettings(acc_id, 0, acc_inf.c_str()), esCreated));

	return acc_id;
}

void AccEditMgr::DeleteAccount(int acc_id)
{
	int idx = FindAccIdx(acc_id);
	bool is_acc_old = (esNone == accounts[idx].second) || (esModified == accounts[idx].second);
	if (is_acc_old) {
		accounts[idx].second = esDeleted;
	} else {
		accounts.erase(accounts.begin() + idx);
	}
}

std::unordered_map<AccEditMgr::EditState, int> AccEditMgr::GetEditState()
{
	std::unordered_map<AccEditMgr::EditState, int> result;
	for (auto acc : accounts) {
		if (esNone != acc.second) {
			++result[acc.second];
		}
	}
	return result;
}

int AccEditMgr::ApplyChanges()
{
	int result = 0;
	for (auto acc : accounts) {
		int acc_res = 0;
		switch (acc.second) {
		case esCreated: acc_res = InitAccResources(acc.first); break;
		case esDeleted: acc_res = DeleteAccResources(acc.first); break;
		}
		if (acc_res < 0) result = acc_res;
	}

	if (result >= 0) result = SaveAccounts();
	return result;
}

int AccEditMgr::InitAccResources(const AccountSettings& acc)
{
	auto store_path = MailMsgStore::GetStorePath(AppCfg.Get().AppDataDir.c_str(), acc.Directory.c_str());
	MailMsgStore mail_store;
	int result = mail_store.SetLocation(store_path.c_str(), acc.Id);
	if (result >= 0)
		logger->LogFmt(llInfo, Log_Scope " acc#%i resources initialized.", acc.Id);
	else
		logger->LogFmt(llError, Log_Scope " acc#%i initialization failed: err=%i.", acc.Id, result);
	return result;
}

int AccEditMgr::DeleteAccResources(const AccountSettings& acc)
{
	auto store_path = MailMsgStore::GetStorePath(AppCfg.Get().AppDataDir.c_str(), acc.Directory.c_str());
	MailMsgStore mail_store;
	int result = mail_store.SetLocation(store_path.c_str(), acc.Id);
	if (result >= 0) result = mail_store.DeleteAll();
	if (result >= 0)
		logger->LogFmt(llInfo, Log_Scope " acc#%i resources deleted.", acc.Id);
	else
		logger->LogFmt(llError, Log_Scope " acc#%i cleanup failed: err=%i.", acc.Id, result);
	return result;
}

int AccEditMgr::SaveAccounts()
{
	std::vector<AccountSettings> save_items;
	std::vector<int> del_ids;
	for (size_t i = 0; i < accounts.size(); ++i) {
		if (esDeleted != accounts[i].second) {
			save_items.push_back(accounts[i].first);
		} else {
			del_ids.push_back(accounts[i].first.Id);
		}
	}
	int result = AccCfg.Save(save_items.data(), save_items.size(), del_ids.data(), del_ids.size());
	if (result < 0) {
		logger->LogFmt(llError, Log_Scope " failed to save accounts: %i.", result);
	} else if (result < save_items.size()) {
		logger->LogFmt(llError, Log_Scope " some accounts (%i of %i) couldn't be saved.",
			(int)(save_items.size() - result), (int)save_items.size());
		result = -1; // ERROR: some of the accounts seems not saved
	} else {
		logger->LogFmt(llInfo, Log_Scope " saved %i account(s), %i deleted.", result, (int)del_ids.size());
	}
	return result;
}
