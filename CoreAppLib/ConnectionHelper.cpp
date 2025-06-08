#include "ConnectionHelper.h"
#include <cstring>
#include <LisCommon/StrUtils.h>

using namespace Connections;
namespace ConnectionHelper_Imp
{
	const int ProtCount = 3;
	const ProtocolType ProtTypes[ProtCount] = { cptNone, cptPop3, cptSmtp };
	const char* ProtNames[ProtCount] = { nullptr, "POP3", "SMTP" };

	const int AuthCount = 4;
	const AuthenticationType AuthTypes[AuthCount] = { catNone, catUserPswd, catPlain, catOAuth2 };
	const char* AuthNames[AuthCount] = { nullptr, "UserPswd", "Plain", "OAuth2" };

	const char AuthSpecSeparator = '.';
	const int AuthSpecSepLen = sizeof(AuthSpecSeparator);
}
using namespace ConnectionHelper_Imp;

ProtocolType ConnectionHelper::GetProtocolType(const char* prot_name)
{
	for (int i = 1; i < ProtCount; ++i)
		if (0 == LisStr::StrICmp(ProtNames[i], prot_name))
			return ProtTypes[i];

	return cptNone;
}

const char* ConnectionHelper::GetProtocolName(ProtocolType prot_type)
{
	for (int i = 1; i < ProtCount; ++i)
		if (prot_type == ProtTypes[i])
			return ProtNames[i];

	return nullptr;
}

Connections::AuthenticationType ConnectionHelper::GetAuthenticationType(const char* auth_name, std::string* auth_spec)
{
	for (int i = 1; i < AuthCount; ++i)
		if (auth_name == LisStr::StrIStr(auth_name, AuthNames[i])) {
			if (auth_spec) {
				size_t len0 = strlen(AuthNames[i]);
				size_t lenX = strlen(auth_name);
				if (lenX > len0) {
					if (auth_name[len0] == AuthSpecSeparator)
						*auth_spec = &auth_name[len0 + AuthSpecSepLen];
					else
						continue;
				}
			}
			return AuthTypes[i];
		}
	return catNone;
}

std::string ConnectionHelper::GetAuthenticationName(Connections::AuthenticationType auth_type, const char* auth_spec)
{
	std::string result;
	for (int i = 1; i < AuthCount; ++i)
		if (auth_type == AuthTypes[i]) {
			result = AuthNames[i];
			if (auth_spec && strlen(auth_spec)) {
				result += AuthSpecSeparator;
				result += auth_spec;
			}
		}

	return result;
}

std::string ConnectionHelper::GetUrl(ConnectionInfo connection)
{
	std::string result;
	result = GetProtocolName(connection.Protocol);
	result += (connection.IsSsl ? "s" : "");
	result += "://";
	result += connection.Server;
	result += ":";
	result += std::to_string(connection.Port);
	return result;
}
