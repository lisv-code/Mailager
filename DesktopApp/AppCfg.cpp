#include "AppCfg.h"
#include <memory>
#include <wx/fileconf.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include "../CoreNetLib/NetLibResources.h"
#include "../CoreAppLib/AppDef.h"

#define CfgFileExtension "cfg"

#define CfgGrp_General "General"
#define CfgPrm_GlobalConfigDir "GlobalConfigDirectory"
#define CfgPrm_UserDataDir "UserDataDirectory"
#define CfgPrm_TempDataDir "TempDataDirectory"
#define CfgPrm_ShareDataDir "ShareDataDirectory"
#define CfgPrm_DefLogLevel "DefaultLogLevel"
#define CfgPrm_MailMsgContentViewer "MailMessageContentViewer"

#define CfgGrp_Network "Network"
#define CfgPrm_NetCABundle "CertificateAuthorityBundle"
#define CfgPrm_SslVerifyPeer "SslVerifyPeer"
#define CfgPrm_SslVerifyHost "SslVerifyHost"
#define CfgPrm_HttpUserAgent "HttpUserAgent"

ApplicationConfiguration AppCfg; // Application Configuration global singleton

namespace ApplicationConfiguration_Imp
{
	static wxString get_cfg_path();

	static void load_cfg_general(wxConfigBase& cfg, ApplicationConfiguration::GeneralSettings& data);
	static void load_cfg_network(wxConfigBase& cfg, NetLibResources::NetworkSettings& data);

	static void to_std_str(std::string& dst, const wxString& src) { dst = src.ToStdString(); }
	static void to_std_str(std::wstring& dst, const wxString& src) { dst = src.ToStdWstring(); }
	static bool to_boolean(const wxString& src);

	static wxString clarify_file_path(const wxString& file_path, const wxString dir_list[], size_t dir_count);
}
using namespace ApplicationConfiguration_Imp;

void ApplicationConfiguration::Load()
{
	auto cfg_path = get_cfg_path();
	wxFileConfig cfg(wxEmptyString, wxEmptyString, cfg_path, wxEmptyString,
		wxCONFIG_USE_RELATIVE_PATH | wxCONFIG_USE_NO_ESCAPE_CHARACTERS);
	wxString txt_buf;
	cfgPath = cfg_path;

	load_cfg_general(cfg, cfgGenData);

	NetLibResources::NetworkSettings net_cfg_data;
	load_cfg_network(cfg, net_cfg_data);
	if (!net_cfg_data.CertificateAuthorityBundle.IsEmpty()) {
		const wxString alt_dirs[] = { cfgGenData.AppExeDir, cfgGenData.ShrDataDir, cfgGenData.UsrDataDir };
		net_cfg_data.CertificateAuthorityBundle.Set(new std::string(clarify_file_path(
			net_cfg_data.CertificateAuthorityBundle.Get(), alt_dirs, sizeof(alt_dirs) / sizeof(wxString))
				.ToStdString()));
	}
	NetLibResources::global_cfg_set(net_cfg_data);
}

// ********************************* ApplicationConfiguration_Imp **********************************

static wxString ApplicationConfiguration_Imp::get_cfg_path()
{
	wxFileName file_path = wxFileName(wxStandardPaths::Get().GetExecutablePath());
	file_path.SetExt(wxT(CfgFileExtension));
	// Check User dir, Config dir, executable dir if no config data in the current working directory
	const wxString alt_dirs[] = {
		wxExpandEnvVars(AppDef_UserDataDir), wxExpandEnvVars(AppDef_GlobalConfigDir), file_path.GetPath()
	};
	return clarify_file_path(file_path.GetFullName(), alt_dirs, sizeof(alt_dirs) / sizeof(wxString));
}

static void ApplicationConfiguration_Imp::load_cfg_general(wxConfigBase& cfg,
	ApplicationConfiguration::GeneralSettings& data)
{
	wxFileName exe_path(wxStandardPaths::Get().GetExecutablePath());
	to_std_str(data.AppExeDir, exe_path.GetPath());

	wxString txt_buf;
	if (!cfg.Read(CfgGrp_General "/" CfgPrm_GlobalConfigDir, &txt_buf))
		txt_buf = AppDef_GlobalConfigDir;
	to_std_str(data.GlobalCfgDir, wxExpandEnvVars(txt_buf));

	if (!cfg.Read(CfgGrp_General "/" CfgPrm_UserDataDir, &txt_buf))
		txt_buf = AppDef_UserDataDir;
	to_std_str(data.UsrDataDir, wxExpandEnvVars(txt_buf));

	if (!cfg.Read(CfgGrp_General "/" CfgPrm_TempDataDir, &txt_buf))
		txt_buf = AppDef_TempDataDir;
	to_std_str(data.TmpDataDir, wxExpandEnvVars(txt_buf));

	if (!cfg.Read(CfgGrp_General "/" CfgPrm_ShareDataDir, &txt_buf))
		txt_buf = AppDef_ShareDataDir;
	to_std_str(data.ShrDataDir, wxExpandEnvVars(txt_buf));

	data.DefaultLogLevel = cfg.ReadLong(CfgGrp_General "/" CfgPrm_DefLogLevel, -1);

	data.MailMessageContentViewer =
		cfg.ReadLong(CfgGrp_General "/" CfgPrm_MailMsgContentViewer, 0);
}

static void ApplicationConfiguration_Imp::load_cfg_network(wxConfigBase& cfg, NetLibResources::NetworkSettings& data)
{
	wxString txt_buf;

	if (cfg.Read(CfgGrp_Network "/" CfgPrm_NetCABundle, &txt_buf))
		data.CertificateAuthorityBundle.Set(new std::string(txt_buf.ToStdString()));

	if (cfg.Read(CfgGrp_Network "/" CfgPrm_SslVerifyPeer, &txt_buf))
		data.SslVerifyPeer.Set(new bool(to_boolean(txt_buf)));

	if (cfg.Read(CfgGrp_Network "/" CfgPrm_SslVerifyHost, &txt_buf))
		data.SslVerifyHost.Set(new bool(to_boolean(txt_buf)));

	if (cfg.Read(CfgGrp_Network "/" CfgPrm_HttpUserAgent, &txt_buf))
		data.HttpUserAgent.Set(new std::string(txt_buf.ToUTF8()));
	else
		data.HttpUserAgent.Set(new std::string(AppDef_NetDefaultUserAgent));
}

static bool ApplicationConfiguration_Imp::to_boolean(const wxString& src)
{
	int val = 0;
	if (src.ToInt(&val)) return static_cast<bool>(val);
	else return src.Strip(wxString::stripType::both).IsSameAs("true", false);
}

static wxString ApplicationConfiguration_Imp::clarify_file_path(const wxString& file_path,
	const wxString dir_list[], size_t dir_count)
{
	if (file_path.IsEmpty()) return wxString();

	wxFileName result(file_path);
	result.Normalize(wxPATH_NORM_ENV_VARS);
	bool file_exists = result.FileExists();

	if (!file_exists && dir_count) {
		auto file_name = result.IsAbsolute() ? result.GetFullName() : result.GetFullPath();
		for (int i = 0; i < dir_count; ++i) {
			result.Assign(dir_list[i], file_name);
			file_exists = result.FileExists();
			if (file_exists) break;
		}
	}

	// Return full path if file has been found, otherwise the original path string
	return file_exists ? result.GetFullPath() : wxString(file_path);
}
