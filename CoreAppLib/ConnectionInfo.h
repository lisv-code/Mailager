#pragma once
#include <string>

namespace Connections
{
	enum ProtocolType { cptNone = 0, cptPop3, cptSmtp };
	enum AuthenticationType { catNone = 0, catUserPswd, catPlain, catOAuth2 };

	struct ConnectionInfo
	{
		ProtocolType Protocol;
		bool IsSsl = false;
		std::string Server;
		unsigned short Port = 0;
		std::string UserName;
		AuthenticationType AuthType;
		std::string AuthSpec;
	};
}
