#pragma once
#include <string>
#include <vector>

struct OAuth2Settings
{
	std::string AuthEndpoint;
	std::string TokenEndpoint;
	std::string Scope;
	std::string ClientId;
	std::string ClientSecret;
};

namespace OAuth2Cfg
{
	std::vector<const char*> GetSpecs();
	OAuth2Settings GetCfg(const char* spec);
}
