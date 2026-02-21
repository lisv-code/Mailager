#include "AccEditMgr.h"
#include <LisCommon/Logger.h>
#include "../../CoreAppLib/AppResCodes.h"
#include "../../CoreAppLib/AccountConfig.h"
#include "../../CoreAppLib/MailMsgStore.h"
#include "../AppCfg.h"
//#include "ListEditMgr.h"

#define DefaultAccDirNamePrefix "account"
#define Log_Scope "AccEdit"

namespace AccEditMgr_Imp
{
	static int get_account_id(const AccountSettings& acc) { return acc.Id; }
	static int init_acc_resources(const AccountSettings& acc, LisLog::ILogger* logger);
	static int delete_acc_resources(const AccountSettings& acc, LisLog::ILogger* logger);
}
using namespace AccEditMgr_Imp;
using namespace LisLog;

AccEditMgr::AccEditMgr() : editMgr(get_account_id) { }

std::unordered_map<int, std::string> AccEditMgr::LoadData()
{
	std::unordered_map<int, std::string> result;
	auto acc_iter = AccCfg.GetIter();
	editMgr.Clear();
	for (auto it = acc_iter.first; it != acc_iter.second; ++it) {
		auto acc = editMgr.AddItem(*it, ListEditMgr_Def::EditState::None);
		result.insert({ acc->Id, acc->GetName() });
	}
	return result;
}

AccountSettings* AccEditMgr::FindAccount(int acc_id)
{
	return editMgr.FindItem(acc_id);
}

bool AccEditMgr::SetAccountModified(int acc_id)
{
	return editMgr.SetItemModified(acc_id);
}

int AccEditMgr::CreateAccount()
{
	int acc_id = AccCfg.GetLastId();
	editMgr.EnumItems([&acc_id](const AccountSettings& acc, ListEditMgr_Def::EditState state)
		{ if (acc_id < acc.Id) acc_id = acc.Id;  return true; });
	++acc_id;

	std::string acc_inf = DefaultAccDirNamePrefix;
	acc_inf += std::to_string(acc_id);
	editMgr.AddItem(AccountSettings(acc_id, 0, acc_inf.c_str()), ListEditMgr_Def::EditState::Created);

	return acc_id;
}

bool AccEditMgr::DeleteAccount(int acc_id)
{
	return editMgr.DeleteItem(acc_id);
}

std::unordered_map<ListEditMgr_Def::EditState, size_t> AccEditMgr::GetAccEditState() const
{
	return editMgr.GetEditState();
}

bool AccEditMgr::ApplyChanges()
{
	std::vector<AccountSettings> save_items;
	std::vector<int> del_ids;
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	bool is_apply_ok = editMgr.ApplyChanges(
		[this, logger](const AccountSettings& acc, ListEditMgr_Def::EditState state) {
			int acc_res = ResCode_Ok;
			switch (state) {
			case ListEditMgr_Def::EditState::Created: acc_res = init_acc_resources(acc, logger); break;
			case ListEditMgr_Def::EditState::Deleted: acc_res = delete_acc_resources(acc, logger); break;
			}
			return acc_res _Is_Ok_ResCode;
		},
		save_items,
		del_ids
	);
	if (!is_apply_ok) return false;

	int res_code = AccCfg.Save(save_items.data(), save_items.size(), del_ids.data(), del_ids.size());
	if (res_code _Is_Err_ResCode) {
		logger->LogFmt(llError, Log_Scope " failed to save accounts: %i.", res_code);
	} else if (res_code < save_items.size()) {
		logger->LogFmt(llError, Log_Scope " some accounts (%i of %i) couldn't be saved.",
			(int)(save_items.size() - res_code), (int)save_items.size());
		res_code = Error_Gen_Undefined; // ERROR: some of the accounts seem not saved
	} else {
		logger->LogFmt(llInfo, Log_Scope " saved %i account(s), %i deleted.", res_code, (int)del_ids.size());
	}
	return res_code _Is_Ok_ResCode;
}

// **************************************** AccEditMgr_Imp *****************************************

int AccEditMgr_Imp::init_acc_resources(const AccountSettings& acc, LisLog::ILogger* logger)
{
	auto store_path = MailMsgStore::GetStoreDirPath(AppCfg.GetGeneral().UsrDataDir.c_str(), acc.Directory.c_str());
	MailMsgStore mail_store;
	int result = mail_store.SetLocation(store_path.c_str(), acc.Id);
	if (result _Is_Ok_ResCode)
		logger->LogFmt(llInfo, Log_Scope " acc#%i resources initialized.", acc.Id);
	else
		logger->LogFmt(llError, Log_Scope " acc#%i initialization failed: err=%i.", acc.Id, result);
	return result;
}

int AccEditMgr_Imp::delete_acc_resources(const AccountSettings& acc, LisLog::ILogger* logger)
{
	auto store_path = MailMsgStore::GetStoreDirPath(AppCfg.GetGeneral().UsrDataDir.c_str(), acc.Directory.c_str());
	MailMsgStore mail_store;
	int result = mail_store.SetLocation(store_path.c_str(), acc.Id);
	if (result _Is_Ok_ResCode) result = mail_store.DeleteAll();
	if (result _Is_Ok_ResCode)
		logger->LogFmt(llInfo, Log_Scope " acc#%i resources deleted.", acc.Id);
	else
		logger->LogFmt(llError, Log_Scope " acc#%i cleanup failed: err=%i.", acc.Id, result);
	return result;
}
