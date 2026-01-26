#include "ConnectionUiCtrlHelper.h"
#include "../../CoreAppLib/OAuth2Config.h"

namespace ConnectionUiCtrlHelper
{
	const int IncProtCount = 2;
	const Connections::ProtocolType IncProtTypes[IncProtCount] = {
		Connections::ProtocolType::cptNone, Connections::ProtocolType::cptPop3 };
	const wxString IncProtNames[IncProtCount] = {
		wxT(""), wxT("POP3 (Post Office Protocol)") };

	const int OutProtCount = 2;
	const Connections::ProtocolType OutProtTypes[OutProtCount] = {
		Connections::ProtocolType::cptNone, Connections::ProtocolType::cptSmtp };
	const wxString OutProtNames[OutProtCount] = {
		wxT(""), wxT("SMTP (Simple Mail Transfer Protocol)") };

	const int DefAuthCount = 4;
	const Connections::AuthenticationType DefAuthTypes[DefAuthCount] = {
		Connections::AuthenticationType::catNone,
		Connections::AuthenticationType::catUserPswd,
		Connections::AuthenticationType::catPlain,
		Connections::AuthenticationType::catOAuth2 };
	const wxString DefAuthNames[DefAuthCount] = {
		wxT(""), wxT("User password"), wxT("Plain"), wxT("OAuth 2") };

	static std::vector<Connections::AuthenticationType> AuthTypes;
	static std::vector<wxString> AuthNames;

	typedef std::vector<const char*> SpecList;
	static void load_auth_data();
	static bool is_auth_type_spec(Connections::AuthenticationType auth_type);
	static SpecList load_auth_spec(Connections::AuthenticationType auth_type);
	const wxString AuthNameSpecSeparator = wxT(" - ");
	static wxString get_auth_name(const wxString name_base, const wxString auth_spec);
	static wxString get_auth_spec(const wxString auth_name);
}
using namespace ConnectionUiCtrlHelper;

void ConnectionUiCtrlHelper::init_prot_items(wxItemContainer* container, bool incoming)
{
	container->Clear();
	auto prot_count = incoming ? IncProtCount : OutProtCount;
	auto prot_names = incoming ? IncProtNames : OutProtNames;
	container->Append(prot_count, prot_names);
}

void ConnectionUiCtrlHelper::init_auth_items(wxItemContainer* container)
{
	load_auth_data();
	container->Clear();
	container->Append(AuthNames.size(), AuthNames.data());
}

int ConnectionUiCtrlHelper::find_prot_item_index(Connections::ProtocolType prot_type, bool incoming)
{
	const int count = incoming ? IncProtCount : OutProtCount;
	const Connections::ProtocolType* items = incoming ? IncProtTypes : OutProtTypes;
	for (int i = 0; i < count; ++i)
		if (prot_type == items[i])
			return i;
	return -1;
}

int ConnectionUiCtrlHelper::find_auth_item_index(Connections::AuthenticationType auth_type, const wxString auth_spec)
{
	for (int i = 0; i < AuthTypes.size(); ++i)
		if (auth_type == AuthTypes[i]) {
			if (is_auth_type_spec(auth_type)) {
				if (get_auth_spec(AuthNames[i]).IsSameAs(auth_spec))
					return i;
			} else
				return i;
		}
	return -1;
}

bool ConnectionUiCtrlHelper::check_text_value_required(const wxString& value)
{
	return !value.Strip().IsEmpty();
}

bool ConnectionUiCtrlHelper::check_port_value(const wxString& value)
{
	if (value.IsEmpty()) return true;
	int num;
	if (value.ToInt(&num)) return (num > 0) && (num < 65536);
	return false;
}

bool ConnectionUiCtrlHelper::set_text_value(std::string& value, const wxTextCtrl* ctrl)
{
	wxCharBuffer new_value = ctrl->GetValue().ToUTF8();
	if (value == new_value.data()) return false;
	value = new_value.data();
	return true;
}

bool ConnectionUiCtrlHelper::set_prot_value(Connections::ProtocolType& value, const wxChoice* ctrl, bool incoming)
{
	const Connections::ProtocolType* prot_types = incoming ? IncProtTypes : OutProtTypes;
	Connections::ProtocolType new_value = prot_types[ctrl->GetSelection()];
	if (value == new_value) return false;
	value = new_value;
	return true;
}

bool ConnectionUiCtrlHelper::set_bool_value(bool& value, const wxCheckBox* ctrl)
{
	bool new_value = ctrl->IsChecked();
	if (value == new_value) return false;
	value = new_value;
	return true;
}

bool ConnectionUiCtrlHelper::set_port_value(unsigned short& value, const wxTextCtrl* ctrl)
{
	unsigned int new_value = 0;
	ctrl->GetValue().ToUInt(&new_value);
	if (value == (unsigned short)new_value) return false;
	value = (unsigned short)new_value;
	return true;
}

bool ConnectionUiCtrlHelper::set_auth_value(Connections::AuthenticationType& auth_type, std::string& auth_spec,
	const wxChoice* ctrl)
{
	int idx = ctrl->GetSelection();
	Connections::AuthenticationType new_auth_type = AuthTypes[idx];
	std::string new_auth_spec;
	if (is_auth_type_spec(new_auth_type)) {
		new_auth_spec = get_auth_spec(ctrl->GetStringSelection()).ToUTF8();
	}
	if ((auth_type == new_auth_type) && (auth_spec == new_auth_spec)) return false;
	auth_type = new_auth_type;
	auth_spec = new_auth_spec;
	return true;
}

static void ConnectionUiCtrlHelper::load_auth_data()
{
	if (AuthTypes.size() > 0) return; // Already loaded

	for (size_t i = 0; i < DefAuthCount; ++i) {
		
		if (is_auth_type_spec(DefAuthTypes[i])) {
			SpecList spec_list = load_auth_spec(DefAuthTypes[i]);
			for (auto spec : spec_list) {
				AuthTypes.push_back(DefAuthTypes[i]);
				AuthNames.push_back(get_auth_name(DefAuthNames[i], spec));
			}
		} else {
			AuthTypes.push_back(DefAuthTypes[i]);
			AuthNames.push_back(DefAuthNames[i]);
		}
	}
}

static bool ConnectionUiCtrlHelper::is_auth_type_spec(Connections::AuthenticationType auth_type)
{
	return Connections::AuthenticationType::catOAuth2 == auth_type;
}

static SpecList ConnectionUiCtrlHelper::load_auth_spec(Connections::AuthenticationType auth_type)
{
	switch (auth_type) {
	case Connections::AuthenticationType::catOAuth2:
		return OAuth2Cfg::GetSpecs();
	default:
		return SpecList();
	}
}

static wxString ConnectionUiCtrlHelper::get_auth_name(const wxString name_base, const wxString auth_spec)
{
	return name_base + AuthNameSpecSeparator + auth_spec;
}

static wxString ConnectionUiCtrlHelper::get_auth_spec(const wxString auth_name)
{
	int pos = auth_name.Find(AuthNameSpecSeparator);
	if (pos > 0)
		return auth_name.Right(auth_name.Length() - pos - AuthNameSpecSeparator.Length());
	else
		return wxString();
}
