#pragma once
#include <vector>
#include <utility>
#include <LisCommon/FileSystem.h>
#include "OAuth2Settings.h"

class OAuth2Config
{
public:
	struct GeneralSettings {
		bool VerificationRequired;
	};
	typedef std::vector<OAuth2ProviderSettings> ProviderSettingsCollection;
	typedef ProviderSettingsCollection::const_iterator ProvidersIterator;

	OAuth2Config();

	const FILE_PATH_CHAR* GetCfgPath() const;
	void SetCfgPath(const FILE_PATH_CHAR* file_path);
	int Load(const FILE_PATH_CHAR* alt_file_paths[], size_t alt_path_count);
	int Save(GeneralSettings* gen_cfg, OAuth2ProviderSettings* prov_items, size_t prov_count);

	const GeneralSettings& GetGeneral() const;
	std::pair<ProvidersIterator, ProvidersIterator> GetProvIter() const;
	const OAuth2ProviderSettings* FindProvider(const char* name) const;

private:
	std::basic_string<FILE_PATH_CHAR> filePath;
	GeneralSettings genCfg;
	ProviderSettingsCollection providers;
};

extern OAuth2Config OAuth2Cfg; // OAuth2 Configuration global singleton
