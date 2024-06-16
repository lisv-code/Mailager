#pragma once
#include <string>
#include "../../CoreAppLib/ConnectionInfo.h"
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/textctrl.h>

namespace ConnectionUiCtrlHelper
{
	void init_prot_items(wxItemContainer* container, bool incoming);
	void init_auth_items(wxItemContainer* container);

	int find_prot_item_index(Connections::ProtocolType prot_type, bool incoming);
	int find_auth_item_index(Connections::AuthenticationType auth_type, const wxString auth_spec);

	bool check_port_value(const wxString& value);

	bool set_text_value(std::string& value, const wxTextCtrl* ctrl);
	bool set_prot_value(Connections::ProtocolType& value, const wxChoice* ctrl, bool incoming);
	bool set_bool_value(bool& value, const wxCheckBox* ctrl);
	bool set_port_value(unsigned short& value, const wxTextCtrl* ctrl);
	bool set_auth_value(Connections::AuthenticationType& auth_type, std::string& auth_spec,
		const wxChoice* ctrl);
}
