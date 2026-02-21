#pragma once
#include <utility>
#include <vector>
#include "../../CoreAppLib/OAuth2Settings.h"
#include "../../CoreAppLib/ListEditMgr.h"

class OAuth2EditMgr
{
public:
	struct GeneralSettings {
		bool VerificationRequired;
	};

	OAuth2EditMgr();
	// Loads config data and returns general settings and providers names
	void LoadData(GeneralSettings& gen_cfg, std::vector<std::string>& prov_names);

	OAuth2ProviderSettings* FindProvider(const char* name, const OAuth2ProviderSettings* skip_item = nullptr);
	bool SetProviderModified(const char* name);
	std::string CreateProvider();
	bool DeleteProvider(const char* name);
	std::unordered_map<ListEditMgr_Def::EditState, size_t> GetProvEditState() const;

	bool ApplyChanges();

private:
	ListEditMgr<OAuth2ProviderSettings, std::string> provListMgr;
};
