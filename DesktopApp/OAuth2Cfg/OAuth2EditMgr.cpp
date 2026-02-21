#include "OAuth2EditMgr.h"
#include <LisCommon/Logger.h>
#include "../../CoreAppLib/AppResCodes.h"
#include "../../CoreAppLib/OAuth2Config.h"

#define Log_Scope "OA2Edit"
#define NewProvNamePrefix "new_provider_"

namespace OAuth2EditMgr_Imp
{
	static std::string get_provider_id(const OAuth2ProviderSettings& prov) { return prov.Name; }
	static std::string generate_new_prov_name(ListEditMgr<OAuth2ProviderSettings, std::string>& prov_list_mgr);
}
using namespace OAuth2EditMgr_Imp;
using namespace LisLog;

OAuth2EditMgr::OAuth2EditMgr() : provListMgr(get_provider_id) { }

void OAuth2EditMgr::LoadData(GeneralSettings& gen_cfg, std::vector<std::string>& prov_names)
{
	gen_cfg.VerificationRequired = OAuth2Cfg.GetGeneral().VerificationRequired;

	auto prov_iter = OAuth2Cfg.GetProvIter();
	for (auto it = prov_iter.first; it != prov_iter.second; ++it) {
		provListMgr.AddItem(*it, ListEditMgr_Def::EditState::None);
		prov_names.push_back(it->Name);
	}
}

OAuth2ProviderSettings* OAuth2EditMgr::FindProvider(const char* name, const OAuth2ProviderSettings* skip_item)
{
	return provListMgr.FindItem(name, skip_item);
}

bool OAuth2EditMgr::SetProviderModified(const char* name)
{
	return provListMgr.SetItemModified(name);
}

std::string OAuth2EditMgr::CreateProvider()
{
	auto prov_name = generate_new_prov_name(provListMgr);
	auto prov_data = provListMgr.AddItem(
		OAuth2ProviderSettings(prov_name.c_str(), OAuth2ProviderStatus_Disabled),
		ListEditMgr_Def::EditState::Created);
	return get_provider_id(*prov_data);
}

bool OAuth2EditMgr::DeleteProvider(const char* name)
{
	return provListMgr.DeleteItem(name);
}

std::unordered_map<ListEditMgr_Def::EditState, size_t> OAuth2EditMgr::GetProvEditState() const
{
	return provListMgr.GetEditState();
}

bool OAuth2EditMgr::ApplyChanges()
{
	std::vector<OAuth2ProviderSettings> save_items;
	std::vector<std::string> del_ids;
	bool is_apply_ok = provListMgr.ApplyChanges(nullptr, save_items, del_ids);
	if (!is_apply_ok) return false;

	// TODO: provide general settings to save
	int res_code = OAuth2Cfg.Save(nullptr, save_items.data(), save_items.size());
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	if (res_code _Is_Err_ResCode) {
		logger->LogFmt(llError, Log_Scope " failed to save OAuth2 configuration: %i.", res_code);
	} else if (res_code < save_items.size()) {
		logger->LogFmt(llError, Log_Scope " some providers (%i of %i) couldn't be saved.",
			(int)(save_items.size() - res_code), (int)save_items.size());
		res_code = Error_Gen_Undefined; // ERROR: some of the providers seem not saved
	} else {
		logger->LogFmt(llInfo, Log_Scope " saved %i provider(s), %i deleted.", res_code, (int)del_ids.size());
	}
	return res_code _Is_Ok_ResCode;
}

// *************************************** OAuth2EditMgr_Imp ***************************************

static std::string OAuth2EditMgr_Imp::generate_new_prov_name(
	ListEditMgr<OAuth2ProviderSettings, std::string>& prov_list_mgr)
{
	size_t idx = prov_list_mgr.GetItemCount() + 1;
	while (idx > 0) {
		std::string name(NewProvNamePrefix);
		name += std::to_string(idx);
		bool is_name_ok = prov_list_mgr.EnumItems(
			[&name](const OAuth2ProviderSettings& prov, ListEditMgr_Def::EditState state) {
				return prov.Name != name;
			});
		if (is_name_ok) // The name is checked for uniqueness
			return name;
		++idx;
	}
	return std::string();
}
