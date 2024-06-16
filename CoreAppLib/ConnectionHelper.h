#pragma once
#include <string>
#include "ConnectionInfo.h"

#define Connection_Result_Ok 0

#define Connection_Error_Protocol -1
#define Connection_Error_Handshake -2
#define Connection_Error_Authentication -3

#define Connection_Error_Interrupted -10

class ConnectionHelper
{
public:
	static Connections::ProtocolType GetProtocolType(const char* prot_name);
	static const char* GetProtocolName(Connections::ProtocolType prot_type);

	static Connections::AuthenticationType GetAuthenticationType(const char* auth_name, std::string* auth_spec);
	static std::string GetAuthenticationName(Connections::AuthenticationType auth_type, const char* auth_spec);

	static std::string GetUrl(Connections::ConnectionInfo connection);
};
