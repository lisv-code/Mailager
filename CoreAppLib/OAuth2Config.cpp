#include "OAuth2Config.h"
#include <memory>
#include <cstring>
#include <wx/fileconf.h>
#include <LisCommon/StrUtils.h>

OAuth2Config OAuth2Cfg; // OAuth2 Configuration global singleton

#define CfgGrp_General "General"
#define CfgPrm_VerificationReq "VerificationRequired"

#define CfgGrpPre_Provider "Provider."
#define CfgPrm_Status "Status"
#define CfgPrm_AuthEndpoint "AuthEndpoint"
#define CfgPrm_TokenEndpoint "TokenEndpoint"
#define CfgPrm_Scope "Scope"
#define CfgPrm_ClientId "ClientId"
#define CfgPrm_ClientSecret "ClientSecret"
#define CfgPrm_Comment "Comment"

namespace OAuth2Config_Imp
{
	const size_t CfgSecPreLen_Provider = std::strlen(CfgGrpPre_Provider);

	static std::unique_ptr<wxConfigBase> get_cfg_inst(const wxString& file_name);
	static const FILE_PATH_CHAR* get_cfg_path(const FILE_PATH_CHAR* primary_path,
		const FILE_PATH_CHAR* alt_file_paths[], size_t alt_path_count);
	static void load_general(wxConfigBase& cfg, OAuth2Config::GeneralSettings& data);
	static void load_provider(wxConfigBase& cfg, const wxString& grp, OAuth2ProviderSettings& data);
	static bool save_general(wxConfigBase& cfg, const OAuth2Config::GeneralSettings& data);
	static bool save_provider(wxConfigBase& cfg, const wxString& grp, OAuth2ProviderSettings& data);
}
using namespace OAuth2Config_Imp;

OAuth2Config::OAuth2Config() { }

const FILE_PATH_CHAR* OAuth2Config::GetCfgPath() const
{
	return filePath.c_str();
}

void OAuth2Config::SetCfgPath(const FILE_PATH_CHAR* file_path)
{
	filePath = file_path;
}

int OAuth2Config::Load(const FILE_PATH_CHAR* alt_file_paths[], size_t alt_path_count)
{
	providers.clear();
	auto cfg = get_cfg_inst(get_cfg_path(filePath.c_str(), alt_file_paths, alt_path_count));

	load_general(*cfg, genCfg);

	wxString grp_name;
	long grp_idx = 0;
	bool has_grp = cfg->GetFirstGroup(grp_name, grp_idx);
	while (has_grp) {
		if (grp_name.StartsWith(CfgGrpPre_Provider)) {
			OAuth2ProviderSettings item(
				grp_name.Right(grp_name.Length() - CfgSecPreLen_Provider).ToUTF8(),
				OAuth2ProviderStatus_Disabled);
			load_provider(*cfg, grp_name, item);
			providers.push_back(item);
		}
		has_grp = cfg->GetNextGroup(grp_name, grp_idx);
	}

	return providers.size();
}

int OAuth2Config::Save(GeneralSettings* gen_cfg, OAuth2ProviderSettings* prov_items, size_t prov_count)
{
	auto cfg = get_cfg_inst(filePath);
	cfg->DeleteAll();

	int result = -1;
	GeneralSettings gen_set{ };
	if (gen_cfg) {
		gen_set.VerificationRequired = gen_cfg->VerificationRequired;
	}
	if (save_general(*cfg, gen_set)) {
		providers.clear();
		for (size_t i = 0; i < prov_count; ++i) {
			wxString grp_name(CfgGrpPre_Provider);
			grp_name += prov_items[i].Name;
			bool is_saved = save_provider(*cfg, grp_name, prov_items[i]);
			if (is_saved) {
				providers.push_back(prov_items[i]);
			}
		}
		result = (int)providers.size();
	}
	return result;
}

const OAuth2Config::GeneralSettings& OAuth2Config::GetGeneral() const
{
	return this->genCfg;
}

std::pair<OAuth2Config::ProvidersIterator, OAuth2Config::ProvidersIterator> OAuth2Config::GetProvIter() const
{
	return std::make_pair(providers.begin(), providers.end());
}

const OAuth2ProviderSettings* OAuth2Config::FindProvider(const char* name) const
{
	for (const auto& prov : providers)
		if (prov.Name == name)
			return &prov;
	return nullptr;
}

// *************************************** OAuth2Config_Imp ****************************************

static std::unique_ptr<wxConfigBase> OAuth2Config_Imp::get_cfg_inst(const wxString& file_name)
{
	return std::unique_ptr<wxConfigBase>(new wxFileConfig(wxEmptyString, wxEmptyString, file_name,
		wxEmptyString, wxCONFIG_USE_RELATIVE_PATH | wxCONFIG_USE_NO_ESCAPE_CHARACTERS));
}

static const FILE_PATH_CHAR* OAuth2Config_Imp::get_cfg_path(const FILE_PATH_CHAR* primary_path,
	const FILE_PATH_CHAR* alt_file_paths[], size_t alt_path_count)
{
	const FILE_PATH_CHAR* chk_path = primary_path;
	int chk_idx = 0;
	do {
		wxFileName fnx(chk_path);
		if (fnx.FileExists() || !alt_file_paths) return chk_path;
		chk_path = alt_file_paths[chk_idx++]; // The last read here may be out of array bounds, but the value is not dereferenced
	} while (chk_idx <= alt_path_count);
	return primary_path;
}

static void OAuth2Config_Imp::load_general(wxConfigBase& cfg, OAuth2Config::GeneralSettings& data)
{
	wxString txt_buf;
	if (cfg.Read(CfgGrp_General "/" CfgPrm_VerificationReq, &txt_buf)) {
		int int_val = 0;
		txt_buf.ToInt(&int_val);
		data.VerificationRequired = (bool)int_val;
	}
}

static void OAuth2Config_Imp::load_provider(wxConfigBase& cfg, const wxString& grp, OAuth2ProviderSettings& data)
{
	wxString txt_buf;
	if (cfg.Read(grp + "/" CfgPrm_Status, &txt_buf)) txt_buf.ToInt(&data.Status);
	if (cfg.Read(grp + "/" CfgPrm_AuthEndpoint, &txt_buf)) data.AuthEndpoint = txt_buf;
	if (cfg.Read(grp + "/" CfgPrm_TokenEndpoint, &txt_buf)) data.TokenEndpoint = txt_buf;
	if (cfg.Read(grp + "/" CfgPrm_Scope, &txt_buf)) data.Scope = txt_buf;
	if (cfg.Read(grp + "/" CfgPrm_ClientId, &txt_buf)) data.ClientId = txt_buf;
	if (cfg.Read(grp + "/" CfgPrm_ClientSecret, &txt_buf)) data.ClientSecret = txt_buf;
	if (cfg.Read(grp + "/" CfgPrm_Comment, &txt_buf)) data.Comment = txt_buf;
}

static bool OAuth2Config_Imp::save_general(wxConfigBase& cfg, const OAuth2Config::GeneralSettings& data)
{
	bool result = true;
	result = result && cfg.Write(CfgGrp_General "/" CfgPrm_VerificationReq, data.VerificationRequired);
	return result;
}

static bool OAuth2Config_Imp::save_provider(wxConfigBase& cfg, const wxString& grp, OAuth2ProviderSettings& data)
{
	bool result = true;
	result = result && cfg.Write(grp + "/" CfgPrm_Status, data.Status);
	result = result && cfg.Write(grp + "/" CfgPrm_AuthEndpoint, data.AuthEndpoint.c_str());
	result = result && cfg.Write(grp + "/" CfgPrm_TokenEndpoint, data.TokenEndpoint.c_str());
	result = result && cfg.Write(grp + "/" CfgPrm_Scope, data.Scope.c_str());
	result = result && cfg.Write(grp + "/" CfgPrm_ClientId, data.ClientId.c_str());
	result = result && cfg.Write(grp + "/" CfgPrm_ClientSecret, data.ClientSecret.c_str());
	result = result && cfg.Write(grp + "/" CfgPrm_Comment, data.Comment.c_str());
	return result;
}
