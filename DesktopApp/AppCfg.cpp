#include "AppCfg.h"
#include <memory>
#include <wx/fileconf.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include "../CoreAppLib/AppDef.h"

#define CfgFileExtension "cfg"

#define CfgSec_General "General"
#define CfgPrm_AppDataDir "ApplicationDataDirectory"
#define CfgPrm_TmpDataDir "TemporaryDataDirectory"
#define CfgPrm_DefLogLevel "DefaultLogLevel"
#define CfgPrm_MailMsgContentViewer "MailMessageContentViewer"
#define CfgPrm_NetUserAgent "NetworkUserAgent"

ApplicationConfiguration AppCfg; // Application Configuration global singleton

namespace ApplicationConfiguration_Imp
{
	wxConfigBase* get_cfg();
	void to_std_str(std::string& dst, const wxString& src) { dst = src.ToStdString(); }
	void to_std_str(std::wstring& dst, const wxString& src) { dst = src.ToStdWstring(); }
}
using namespace ApplicationConfiguration_Imp;

void ApplicationConfiguration::Load()
{
	std::unique_ptr<wxConfigBase> cfg(get_cfg());
	wxString txt_buf;

	if (cfg->Read(CfgSec_General "/" CfgPrm_AppDataDir, &txt_buf))
		to_std_str(data.AppDataDir, txt_buf);
	else
		to_std_str(data.AppDataDir, wxExpandEnvVars(AppDef_AppDataDir));

	if (cfg->Read(CfgSec_General "/" CfgPrm_TmpDataDir, &txt_buf))
		to_std_str(data.TmpDataDir, txt_buf);
	else
		to_std_str(data.TmpDataDir, wxExpandEnvVars(AppDef_TmpDataDir));

	data.DefaultLogLevel = cfg->ReadLong(CfgSec_General "/" CfgPrm_DefLogLevel, -1);

	data.MailMessageContentViewer =
		cfg->ReadLong(CfgSec_General "/" CfgPrm_MailMsgContentViewer, 0);

	if (cfg->Read(CfgSec_General "/" CfgPrm_NetUserAgent, &txt_buf))
		to_std_str(data.NetUserAgent, txt_buf);
	else
		data.NetUserAgent = AppDef_NetDefaultUserAgent;
}

wxConfigBase* ApplicationConfiguration_Imp::get_cfg()
{
	wxFileName file_path = wxFileName(wxStandardPaths::Get().GetExecutablePath());
	file_path.SetExt(wxT(CfgFileExtension));
	return new wxFileConfig(wxEmptyString, wxEmptyString, file_path.GetFullPath(),
		wxEmptyString, wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS);
}
