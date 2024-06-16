#pragma once
#include <string>
#include <vector>

struct OAuth2Settings
{
	std::string code_server;
	std::string token_server;
	std::string scope;
	std::string client_id;
	std::string client_secret;
};

namespace OAuth2Cfg
{
	std::vector<const char*> GetSpecs();
	OAuth2Settings GetCfg(const char* spec);
}
