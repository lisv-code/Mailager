#include "AccountConfig.h"
#include <memory>
#include <wx/fileconf.h>
#include "ConnectionHelper.h"

AccountConfig AccCfg; // Account Configuration global singleton

#define CfgGrp_General "General"
#define CfgPrm_AccLastId "AccountLastId"

#define CfgGrpFmt_AccountGeneral "Account.%u.General"
#define CfgGrpFmt_AccountIncoming "Account.%u.Incoming"
#define CfgGrpFmt_AccountOutgoing "Account.%u.Outgoing"
#define CfgGrpFmt_MaxLen 30

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

namespace AccountConfig_Imp
{
	struct GeneralSettings {
		long AccountLastId;
	};

	static std::unique_ptr<wxConfigBase> get_cfg(const FILE_PATH_CHAR* file_path);

	static GeneralSettings load_general(wxConfigBase& cfg, const char* grp);
	static bool save_general(wxConfigBase& cfg, const char* grp, const GeneralSettings& data);

	static bool load_account(wxConfigBase& cfg, int acc_id, AccountSettings& data);
	static void load_acc_general(wxConfigBase& cfg, const char* grp, AccountSettings& data);
	static void load_acc_connection(wxConfigBase& cfg, const char* grp,
		Connections::ConnectionInfo& data);

	static bool save_account(wxConfigBase& cfg, const AccountSettings& data);
	static bool save_acc_general(wxConfigBase& cfg, const char* grp, const AccountSettings& data);
	static bool save_acc_connection(wxConfigBase& cfg, const char* grp,
		const Connections::ConnectionInfo& data);

	static void delete_account(wxConfigBase& cfg, int acc_id);
}
using namespace AccountConfig_Imp;

AccountConfig::AccountConfig() : accLastId(0) { }

const FILE_PATH_CHAR* AccountConfig::GetCfgPath() const
{
	return filePath.c_str();
}

void AccountConfig::SetCfgPath(const FILE_PATH_CHAR* file_path)
{
	filePath = file_path;
}

int AccountConfig::Load()
{
	auto cfg(get_cfg(filePath.c_str()));
	accounts.clear();

	auto gen_set = load_general(*cfg, CfgGrp_General);
	accLastId = gen_set.AccountLastId;

	for (unsigned int id = 1; id <= accLastId; ++id) {
		AccountSettings item(id);
		bool has_data = load_account(*cfg, id, item);
		if (has_data) {
			accounts.push_back(item);
		}
	}

	return (int)accounts.size();
}

int AccountConfig::Save(AccountSettings* save_items, size_t save_count, int* del_ids, size_t del_count)
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
	gen_set.AccountLastId = accLastId;
	if (save_general(*cfg, CfgGrp_General, gen_set))
	{
		accounts.clear();
		for (size_t i = 0; i < save_count; ++i) {
			bool is_saved = save_account(*cfg, save_items[i]);
			if (is_saved) {
				accounts.push_back(save_items[i]);
			}
		}
		result = (int)accounts.size();

		for (size_t i = 0; i < del_count; ++i) {
			delete_account(*cfg, del_ids[i]);
		}

		if (result >= 0) {
			AccountConfig_EventData evt_data;
			for (const auto& acc : accounts) evt_data.Accounts.push_back(&acc);
			if (del_count > 0) evt_data.DeletedAccIds.assign(del_ids, del_ids + del_count);
			RaiseEvent(AccountConfig_EventType::AccountsChanged, evt_data);
		}
	}

	return result;
}

unsigned int AccountConfig::GetLastId() const
{
	return accLastId;
}

std::pair<AccountConfig::AccountsIterator, AccountConfig::AccountsIterator> AccountConfig::GetIter() const
{
	return std::make_pair(accounts.begin(), accounts.end());
}

const AccountSettings* AccountConfig::FindAccount(int acc_id) const
{
	for (const auto& acc : accounts)
		if (acc_id == acc.Id)
			return &acc;
	return nullptr;
}

// ******************************* Internal functions implementation *******************************

static std::unique_ptr<wxConfigBase> AccountConfig_Imp::get_cfg(const FILE_PATH_CHAR* file_path)
{
	return std::unique_ptr<wxConfigBase>(new wxFileConfig(wxEmptyString, wxEmptyString, file_path,
		wxEmptyString, wxCONFIG_USE_RELATIVE_PATH | wxCONFIG_USE_NO_ESCAPE_CHARACTERS));
}

static GeneralSettings AccountConfig_Imp::load_general(wxConfigBase& cfg, const char* grp)
{
	GeneralSettings result;
	wxString txt_buf;
	result.AccountLastId = cfg.ReadLong(wxString(grp) + "/" CfgPrm_AccLastId, 0);
	return result;
}

static bool AccountConfig_Imp::save_general(wxConfigBase& cfg, const char* grp,
	const GeneralSettings& data)
{
	bool result = true;
	result = result && cfg.Write(wxString(grp) + "/" CfgPrm_AccLastId, data.AccountLastId);
	return result;
}

static bool AccountConfig_Imp::load_account(wxConfigBase& cfg, int acc_id, AccountSettings& data)
{
	bool has_data;
	char buf[CfgGrpFmt_MaxLen];
	sprintf(buf, CfgGrpFmt_AccountGeneral, acc_id);
	if (has_data = cfg.Exists(buf))
		load_acc_general(cfg, buf, data);
	if (has_data) {
		sprintf(buf, CfgGrpFmt_AccountIncoming, acc_id);
		if (cfg.Exists(buf)) load_acc_connection(cfg, buf, data.Incoming);
		sprintf(buf, CfgGrpFmt_AccountOutgoing, acc_id);
		if (cfg.Exists(buf)) load_acc_connection(cfg, buf, data.Outgoing);
	}
	return has_data;
}

static void AccountConfig_Imp::load_acc_general(wxConfigBase& cfg, const char* grp, AccountSettings& data)
{
	wxString txt_buf;
	data.Status = cfg.ReadLong(wxString(grp) + "/" CfgPrm_Status, 0);
	if (cfg.Read(wxString(grp) + "/" CfgPrm_Directory, &txt_buf))
		data.Directory = txt_buf.ToStdString();
	if (cfg.Read(wxString(grp) + "/" CfgPrm_AccountName, &txt_buf))
		data.AccountName = txt_buf.ToStdString();
	if (cfg.Read(wxString(grp) + "/" CfgPrm_EmailAddress, &txt_buf))
		data.EMailAddress = txt_buf.ToStdString();
}

static void AccountConfig_Imp::load_acc_connection(wxConfigBase& cfg, const char* grp,
	Connections::ConnectionInfo& data)
{
	wxString txt_buf;
	if (cfg.Read(wxString(grp) + "/" + CfgPrm_Protocol, &txt_buf))
		data.Protocol = ConnectionHelper::GetProtocolType(txt_buf);
	data.IsSsl = cfg.ReadBool(wxString(grp) + "/" CfgPrm_Ssl, false);
	if (cfg.Read(wxString(grp) + "/" CfgPrm_Server, &txt_buf))
		data.Server = txt_buf.ToStdString();
	data.Port = cfg.ReadLong(wxString(grp) + "/" CfgPrm_Port, 0);
	if (cfg.Read(wxString(grp) + "/" CfgPrm_UserName, &txt_buf))
		data.UserName = txt_buf.ToStdString();
	if (cfg.Read(wxString(grp) + "/" CfgPrm_Auth, &txt_buf))
		data.AuthType = ConnectionHelper::GetAuthenticationType(txt_buf, &data.AuthSpec);
}

static bool AccountConfig_Imp::save_account(wxConfigBase& cfg, const AccountSettings& data)
{
	bool is_saved = false;
	char buf[CfgGrpFmt_MaxLen];
	sprintf(buf, CfgGrpFmt_AccountGeneral, data.Id);
	is_saved = save_acc_general(cfg, buf, data);
	if (is_saved) {
		sprintf(buf, CfgGrpFmt_AccountIncoming, data.Id);
		save_acc_connection(cfg, buf, data.Incoming);
		sprintf(buf, CfgGrpFmt_AccountOutgoing, data.Id);
		save_acc_connection(cfg, buf, data.Outgoing);
	}
	return is_saved;
}

static bool AccountConfig_Imp::save_acc_general(wxConfigBase& cfg, const char* grp,
	const AccountSettings& data)
{
	bool result = true;
	result = result && cfg.Write(wxString(grp) + "/" CfgPrm_Status, data.Status);
	result = result && cfg.Write(wxString(grp) + "/" CfgPrm_Directory, data.Directory.c_str());
	result = result && cfg.Write(wxString(grp) + "/" CfgPrm_AccountName, data.AccountName.c_str());
	result = result && cfg.Write(wxString(grp) + "/" CfgPrm_EmailAddress, data.EMailAddress.c_str());
	return result;
}

static bool AccountConfig_Imp::save_acc_connection(wxConfigBase& cfg, const char* grp,
	const Connections::ConnectionInfo& data)
{
	bool result = true;
	result = result && cfg.Write(wxString(grp) + "/" + CfgPrm_Protocol,
		ConnectionHelper::GetProtocolName(data.Protocol));
	result = result && cfg.Write(wxString(grp) + "/" CfgPrm_Ssl, data.IsSsl);
	result = result && cfg.Write(wxString(grp) + "/" CfgPrm_Server, data.Server.c_str());
	result = result && cfg.Write(wxString(grp) + "/" CfgPrm_Port, data.Port);
	result = result && cfg.Write(wxString(grp) + "/" CfgPrm_UserName, data.UserName.c_str());
	result = result && cfg.Write(wxString(grp) + "/" CfgPrm_Auth,
		ConnectionHelper::GetAuthenticationName(data.AuthType, data.AuthSpec.c_str()).c_str());
	return result;
}

static void AccountConfig_Imp::delete_account(wxConfigBase& cfg, int acc_id)
{
	char buf[CfgGrpFmt_MaxLen];
	sprintf(buf, CfgGrpFmt_AccountGeneral, acc_id);
	cfg.DeleteGroup(buf);
	sprintf(buf, CfgGrpFmt_AccountIncoming, acc_id);
	cfg.DeleteGroup(buf);
	sprintf(buf, CfgGrpFmt_AccountOutgoing, acc_id);
	cfg.DeleteGroup(buf);
}
