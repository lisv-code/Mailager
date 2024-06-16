#include "AccountCfg.h"
#include <memory>
#include <wx/fileconf.h>
#include "ConnectionHelper.h"

using namespace AccountCfg_Def;

AccountCfg AccCfg; // Account Configuration global singleton

#define CfgSec_General "General"

#define CfgPrm_AccLastId "AccountLastId"

#define CfgSecFmt_AccountGeneral "Account.%u.General"
#define CfgSecFmt_AccountIncoming "Account.%u.Incoming"
#define CfgSecFmt_AccountOutgoing "Account.%u.Outgoing"

#define CfgPrm_Status "Status"
#define CfgPrm_Directory "Directory"
#define CfgPrm_AccountName "AccountName"
#define CfgPrm_EmailAddress "EMailAddress"

#define CfgPrm_Protocol "Protocol"
#define CfgPrm_Ssl "SSL"
#define CfgPrm_Server "Server"
#define CfgPrm_Port "Port"
#define CfgPrm_UserName "UserName"
#define CfgPrm_Auth "Authentication"

namespace AccountCfg_Imp
{
	struct GeneralSettings {
		long AccounLastId;
	};

	static wxConfigBase* get_cfg(const FILE_PATH_CHAR* file_path);

	static GeneralSettings load_general(wxConfigBase* cfg, const char* section);
	static bool save_general(wxConfigBase* cfg, const char* section, const GeneralSettings& data);

	static bool load_account(wxConfigBase* cfg, int acc_id, AccountSettings& data);
	static void load_acc_general(wxConfigBase* cfg, const char* section, AccountSettings& data);
	static void load_acc_connection(wxConfigBase* cfg, const char* section,
		Connections::ConnectionInfo& data);

	static bool save_account(wxConfigBase* cfg, const AccountSettings& data);
	static bool save_acc_general(wxConfigBase* cfg, const char* section, const AccountSettings& data);
	static bool save_acc_connection(wxConfigBase* cfg, const char* section,
		const Connections::ConnectionInfo& data);

	static void delete_account(wxConfigBase* cfg, int acc_id);
}
using namespace AccountCfg_Imp;

AccountCfg::AccountCfg() : accLastId(0) { }

void AccountCfg::SetFilePath(const FILE_PATH_CHAR* file_path)
{
	filePath = file_path;
}

int AccountCfg::Load()
{
	std::unique_ptr<wxConfigBase> cfg(get_cfg(filePath.c_str()));
	accounts.clear();

	auto gen_set = load_general(cfg.get(), CfgSec_General);
	accLastId = gen_set.AccounLastId;

	for (unsigned int id = 1; id <= accLastId; ++id) {
		AccountSettings item(id);
		bool has_data = load_account(cfg.get(), id, item);
		if (has_data) {
			accounts.push_back(item);
		}
	}

	return (int)accounts.size();
}

int AccountCfg::Save(AccountSettings* save_items, size_t save_count, int* del_ids, size_t del_count)
{
	for (size_t i = 0; i < save_count; ++i) {
		if ((save_items[i].Id > accLastId) || (save_items[i].Id <= 0)) {
			++accLastId;
			save_items[i].Id = accLastId;
		}
	}

	std::unique_ptr<wxConfigBase> cfg(get_cfg(filePath.c_str()));

	int result = -1;
	GeneralSettings gen_set;
	gen_set.AccounLastId = accLastId;
	if (save_general(cfg.get(), CfgSec_General, gen_set))
	{
		accounts.clear();
		for (size_t i = 0; i < save_count; ++i) {
			bool is_saved = save_account(cfg.get(), save_items[i]);
			if (is_saved) {
				accounts.push_back(save_items[i]);
			}
		}
		result = (int)accounts.size();

		for (size_t i = 0; i < del_count; ++i) {
			delete_account(cfg.get(), del_ids[i]);
		}

		if (result >= 0) {
			EventData evt_data;
			for (const auto& acc : accounts) evt_data.Accounts.push_back(&acc);
			if (del_count > 0) evt_data.DeletedAccIds.assign(del_ids, del_ids + del_count);
			RaiseEvent(etAccountsChanged, &evt_data);
		}
	}

	return result;
}

unsigned int AccountCfg::GetLastId() const
{
	return accLastId;
}

void AccountCfg::GetIter(AccountsIterator& begin, AccountsIterator& end) const
{
	begin = accounts.begin();
	end = accounts.end();
}

const AccountSettings* AccountCfg::FindAccount(int acc_id) const
{
	for (const auto& acc : accounts)
		if (acc_id == acc.Id)
			return &acc;
	return nullptr;
}

// ******************************* Internal functions implementation *******************************

static wxConfigBase* AccountCfg_Imp::get_cfg(const FILE_PATH_CHAR* file_path)
{
	return new wxFileConfig(wxEmptyString, wxEmptyString, file_path,
		wxEmptyString, wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_NO_ESCAPE_CHARACTERS);
}

static GeneralSettings AccountCfg_Imp::load_general(wxConfigBase* cfg, const char* section)
{
	GeneralSettings result;
	wxString txt_buf;
	result.AccounLastId = cfg->ReadLong(wxString(section) + "/" CfgPrm_AccLastId, false);
	return result;
}

static bool AccountCfg_Imp::save_general(wxConfigBase* cfg, const char* section,
	const GeneralSettings& data)
{
	bool result = true;
	result = result && cfg->Write(wxString(section) + "/" CfgPrm_AccLastId, data.AccounLastId);
	return result;
}

static bool AccountCfg_Imp::load_account(wxConfigBase* cfg, int acc_id, AccountSettings& data)
{
	bool has_data = true;
	char buf[0xFF];

	sprintf(buf, CfgSecFmt_AccountGeneral, acc_id);
	if (has_data = cfg->Exists(buf)) load_acc_general(cfg, buf, data);

	sprintf(buf, CfgSecFmt_AccountIncoming, acc_id);
	if (has_data = cfg->Exists(buf)) load_acc_connection(cfg, buf, data.Incoming);

	sprintf(buf, CfgSecFmt_AccountOutgoing, acc_id);
	if (has_data |= cfg->Exists(buf)) load_acc_connection(cfg, buf, data.Outgoing);

	return has_data;
}

static void AccountCfg_Imp::load_acc_general(wxConfigBase* cfg, const char* section, AccountSettings& data)
{
	wxString txt_buf;
	data.Status = cfg->ReadLong(wxString(section) + "/" CfgPrm_Status, 0);
	if (cfg->Read(wxString(section) + "/" CfgPrm_Directory, &txt_buf))
		data.Directory = txt_buf.ToStdString();
	if (cfg->Read(wxString(section) + "/" CfgPrm_AccountName, &txt_buf))
		data.AccountName = txt_buf.ToStdString();
	if (cfg->Read(wxString(section) + "/" CfgPrm_EmailAddress, &txt_buf))
		data.EMailAddress = txt_buf.ToStdString();
}

static void AccountCfg_Imp::load_acc_connection(wxConfigBase* cfg, const char* section,
	Connections::ConnectionInfo& data)
{
	wxString txt_buf;
	if (cfg->Read(wxString(section) + "/" + CfgPrm_Protocol, &txt_buf))
		data.Protocol = ConnectionHelper::GetProtocolType(txt_buf);
	data.IsSsl = cfg->ReadBool(wxString(section) + "/" CfgPrm_Ssl, false);
	if (cfg->Read(wxString(section) + "/" CfgPrm_Server, &txt_buf))
		data.Server = txt_buf.ToStdString();
	data.Port = cfg->ReadLong(wxString(section) + "/" CfgPrm_Port, 0);
	if (cfg->Read(wxString(section) + "/" CfgPrm_UserName, &txt_buf))
		data.UserName = txt_buf.ToStdString();
	if (cfg->Read(wxString(section) + "/" CfgPrm_Auth, &txt_buf))
		data.AuthType = ConnectionHelper::GetAuthenticationType(txt_buf, &data.AuthSpec);
}

static bool AccountCfg_Imp::save_account(wxConfigBase* cfg, const AccountSettings& data)
{
	bool is_saved = false;
	char buf[0xFF];

	sprintf(buf, CfgSecFmt_AccountGeneral, data.Id);
	is_saved |= save_acc_general(cfg, buf, data);

	sprintf(buf, CfgSecFmt_AccountIncoming, data.Id);
	is_saved |= save_acc_connection(cfg, buf, data.Incoming);

	sprintf(buf, CfgSecFmt_AccountOutgoing, data.Id);
	is_saved |= save_acc_connection(cfg, buf, data.Outgoing);

	return is_saved;
}

static bool AccountCfg_Imp::save_acc_general(wxConfigBase* cfg, const char* section,
	const AccountSettings& data)
{
	bool result = true;
	result = result && cfg->Write(wxString(section) + "/" CfgPrm_Status, data.Status);
	result = result && cfg->Write(wxString(section) + "/" CfgPrm_Directory, data.Directory.c_str());
	result = result && cfg->Write(wxString(section) + "/" CfgPrm_AccountName, data.AccountName.c_str());
	result = result && cfg->Write(wxString(section) + "/" CfgPrm_EmailAddress, data.EMailAddress.c_str());
	return result;
}

static bool AccountCfg_Imp::save_acc_connection(wxConfigBase* cfg, const char* section,
	const Connections::ConnectionInfo& data)
{
	bool result = true;
	result = result && cfg->Write(wxString(section) + "/" + CfgPrm_Protocol,
		ConnectionHelper::GetProtocolName(data.Protocol));
	result = result && cfg->Write(wxString(section) + "/" CfgPrm_Ssl, data.IsSsl);
	result = result && cfg->Write(wxString(section) + "/" CfgPrm_Server, data.Server.c_str());
	result = result && cfg->Write(wxString(section) + "/" CfgPrm_Port, data.Port);
	result = result && cfg->Write(wxString(section) + "/" CfgPrm_UserName, data.UserName.c_str());
	result = result && cfg->Write(wxString(section) + "/" CfgPrm_Auth,
		ConnectionHelper::GetAuthenticationName(data.AuthType, data.AuthSpec.c_str()).c_str());
	return result;
}

static void AccountCfg_Imp::delete_account(wxConfigBase* cfg, int acc_id)
{
	char buf[0xFF];
	sprintf(buf, CfgSecFmt_AccountGeneral, acc_id);
	cfg->DeleteGroup(buf);
	sprintf(buf, CfgSecFmt_AccountIncoming, acc_id);
	cfg->DeleteGroup(buf);
	sprintf(buf, CfgSecFmt_AccountOutgoing, acc_id);
	cfg->DeleteGroup(buf);
}
